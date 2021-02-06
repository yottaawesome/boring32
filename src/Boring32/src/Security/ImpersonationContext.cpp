#include "pch.hpp"
#include <stdexcept>
#include "include/Error/Win32Error.hpp"
#include "include/Error/NtStatusError.hpp"
#include "include/Security/ImpersonationContext.hpp"

namespace Boring32::Security
{
	ImpersonationContext::~ImpersonationContext()
	{
		Close();
	}

	ImpersonationContext::ImpersonationContext(HANDLE const token)
	:	m_registryHive(nullptr)
	{
		if (token == nullptr || token == INVALID_HANDLE_VALUE)
			throw std::runtime_error(__FUNCSIG__ ": token is invalid");
		if (ImpersonateLoggedOnUser(token) == false)
			throw Error::Win32Error(
				__FUNCSIG__ ": ImpersonateLoggedOnUser() failed",
				GetLastError()
			);
	}

	bool ImpersonationContext::Close()
	{
		if (m_registryHive)
		{
			RegCloseKey(m_registryHive);
			m_registryHive = nullptr;
		}
		return RevertToSelf();
	}

	HKEY ImpersonationContext::GetUserRegistry()
	{
		if (m_registryHive == nullptr)
		{
			LSTATUS status = RegOpenCurrentUser(KEY_READ, &m_registryHive);
			if (status != ERROR_SUCCESS)
				throw Error::NtStatusError(
					__FUNCSIG__ ": RegOpenCurrentUser() failed", status
				);
		}
		return m_registryHive;
	}
}