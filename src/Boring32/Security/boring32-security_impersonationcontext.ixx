export module boring32:security_impersonationcontext;
import boring32.error;
import boring32.shared;

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
			{
				Move(other);
			}
			ImpersonationContext(Win32::HANDLE const token)
			{
				if (!token || token == Win32::InvalidHandleValue)
					throw Error::Boring32Error("token is invalid");
				// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-impersonateloggedonuser
				if (!Win32::ImpersonateLoggedOnUser(token))
				{
					const auto lastError = Win32::GetLastError();
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
					Win32::Winreg::RegCloseKey(m_registryHive);
					m_registryHive = nullptr;
				}
				// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-reverttoself
				return Win32::RevertToSelf();
			}

			virtual Win32::Winreg::HKEY GetUserRegistry()
			{
				if (m_registryHive == nullptr)
				{
					// https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regopencurrentuser
					Win32::LSTATUS status = Win32::Winreg::RegOpenCurrentUser(Win32::_KEY_READ, &m_registryHive);
					if (status != Win32::ErrorCodes::Success)
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
			Win32::Winreg::HKEY m_registryHive = nullptr;
	};
}