export module boring32.security:impersonationcontext;
import <win32.hpp>;
import boring32.error;

export namespace Boring32::Security
{
	// https://docs.microsoft.com/en-us/windows/win32/secauthz/client-impersonation
	class ImpersonationContext
	{
		public:
			virtual ~ImpersonationContext()
			{
				Close();
			}
			ImpersonationContext(const ImpersonationContext&) = delete;
			ImpersonationContext(ImpersonationContext&& other) noexcept
				: m_registryHive(nullptr)
			{
				Move(other);
			}
			ImpersonationContext(HANDLE const token)
				: m_registryHive(nullptr)
			{
				if (!token || token == INVALID_HANDLE_VALUE)
					throw Error::Boring32Error("token is invalid");
				// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-impersonateloggedonuser
				if (!ImpersonateLoggedOnUser(token))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("ImpersonateLoggedOnUser() failed", lastError);
				}
			}

		public:
			virtual ImpersonationContext& operator=(const ImpersonationContext&) = delete;
			virtual ImpersonationContext& operator=(ImpersonationContext&&) noexcept = delete;

		public:
			virtual bool Close()
			{
				if (m_registryHive)
				{
					RegCloseKey(m_registryHive);
					m_registryHive = nullptr;
				}
				// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-reverttoself
				return RevertToSelf();
			}

			virtual HKEY GetUserRegistry()
			{
				if (m_registryHive == nullptr)
				{
					// https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regopencurrentuser
					LSTATUS status = RegOpenCurrentUser(KEY_READ, &m_registryHive);
					if (status != ERROR_SUCCESS)
						throw Error::NTStatusError("RegOpenCurrentUser() failed", status);
				}
				return m_registryHive;
			}

		protected:
			virtual ImpersonationContext& Move(ImpersonationContext& other) noexcept
			{
				m_registryHive = other.m_registryHive;
				other.m_registryHive = nullptr;
				return *this;
			}

		protected:
			HKEY m_registryHive;
	};
}