module;

#include <stdexcept>
#include <source_location>
#include <Windows.h>

module boring32.security.impersonationcontext;
import boring32.error.win32error;
import boring32.error.ntstatuserror;

namespace Boring32::Security
{
	// https://docs.microsoft.com/en-us/windows/win32/secauthz/client-impersonation
	ImpersonationContext::~ImpersonationContext()
	{
		Close();
	}

	ImpersonationContext::ImpersonationContext(HANDLE const token)
	:	m_registryHive(nullptr)
	{
		if (token == nullptr || token == INVALID_HANDLE_VALUE)
			throw std::runtime_error(__FUNCSIG__ ": token is invalid");
		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-impersonateloggedonuser
		if (ImpersonateLoggedOnUser(token) == false)
			throw Error::Win32Error(
				std::source_location::current(),
				"ImpersonateLoggedOnUser() failed",
				GetLastError()
			);
	}

	ImpersonationContext::ImpersonationContext(ImpersonationContext&& other) noexcept
	:	m_registryHive(nullptr)
	{
		Move(other);
	}

	ImpersonationContext& ImpersonationContext::Move(ImpersonationContext& other) noexcept
	{
		m_registryHive = other.m_registryHive;
		other.m_registryHive = nullptr;
		return *this;
	}

	bool ImpersonationContext::Close()
	{
		if (m_registryHive)
		{
			RegCloseKey(m_registryHive);
			m_registryHive = nullptr;
		}
		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-reverttoself
		return RevertToSelf();
	}

	HKEY ImpersonationContext::GetUserRegistry()
	{
		if (m_registryHive == nullptr)
		{
			// https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regopencurrentuser
			LSTATUS status = RegOpenCurrentUser(KEY_READ, &m_registryHive);
			if (status != ERROR_SUCCESS)
				throw Error::NtStatusError(
					std::source_location::current(),
					"RegOpenCurrentUser() failed", 
					status
				);
		}
		return m_registryHive;
	}
}
