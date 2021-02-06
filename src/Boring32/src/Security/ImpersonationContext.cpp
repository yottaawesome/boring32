#include "pch.hpp"
#include <stdexcept>
#include "include/Error/Win32Error.hpp"
#include "include/Security/ImpersonationContext.hpp"

namespace Boring32::Security
{
	ImpersonationContext::~ImpersonationContext()
	{
		Close();
	}

	ImpersonationContext::ImpersonationContext(HANDLE const token)
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
		return RevertToSelf();
	}
}