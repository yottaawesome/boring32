export module boring32:security.impersonationcontext;
import std;
import boring32.win32;
import :error;

export namespace Boring32::Security
{
	// https://docs.microsoft.com/en-us/windows/win32/secauthz/client-impersonation
	struct ImpersonationContext final
	{
		~ImpersonationContext()
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
			if (not token or token == Win32::InvalidHandleValue)
				throw Error::Boring32Error("token is invalid");
			// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-impersonateloggedonuser
			if (not Win32::ImpersonateLoggedOnUser(token))
				throw Error::Win32Error(Win32::GetLastError(), "ImpersonateLoggedOnUser() failed");
		}

		ImpersonationContext& operator=(const ImpersonationContext&) = delete;
		ImpersonationContext& operator=(ImpersonationContext&&) noexcept = delete;

		bool Close()
		{
			if (m_registryHive)
			{
				Win32::Winreg::RegCloseKey(m_registryHive);
				m_registryHive = nullptr;
			}
			// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-reverttoself
			return Win32::RevertToSelf();
		}

		Win32::Winreg::HKEY GetUserRegistry()
		{
			if (not m_registryHive)
			{
				// https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regopencurrentuser
				Win32::LSTATUS status = Win32::Winreg::RegOpenCurrentUser(Win32::_KEY_READ, &m_registryHive);
				if (status != Win32::ErrorCodes::Success)
					throw Error::NTStatusError(status, "RegOpenCurrentUser() failed");
			}
			return m_registryHive;
		}

		private:
		ImpersonationContext& Move(ImpersonationContext& other) noexcept
		{
			m_registryHive = other.m_registryHive;
			other.m_registryHive = nullptr;
			return *this;
		}

		Win32::Winreg::HKEY m_registryHive = nullptr;
	};
}