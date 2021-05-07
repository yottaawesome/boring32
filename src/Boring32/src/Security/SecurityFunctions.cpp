#include "pch.hpp"
#include <stdexcept>
#include <sddl.h>
#include "include/Error/Win32Error.hpp"
#include "include/Security/SecurityFunctions.hpp"
#include "include/Security/Constants.hpp"

namespace Boring32::Security
{
	Raii::Win32Handle GetProcessToken(const HANDLE processHandle, const DWORD desiredAccess)
	{
		if (processHandle == nullptr || processHandle == INVALID_HANDLE_VALUE)
			throw std::invalid_argument(__FUNCSIG__ ": processHandle cannot be null");

		Raii::Win32Handle handle;
		// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-openprocesstoken
		const bool succeeded = OpenProcessToken(
			processHandle,
			desiredAccess, // https://docs.microsoft.com/en-us/windows/win32/secauthz/access-rights-for-access-token-objects
			&handle
		);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__ ": OpenProcessToken() failed");

		return handle;
	}

	void AdjustPrivileges(HANDLE token, const std::wstring& privilege, const bool enabled)
	{
		// See also: https://docs.microsoft.com/en-us/windows/win32/secauthz/enabling-and-disabling-privileges-in-c--
		if (token == nullptr || token == INVALID_HANDLE_VALUE)
			throw std::invalid_argument(__FUNCSIG__ ": token cannot be null");

		// https://docs.microsoft.com/en-us/windows/win32/secauthz/privilege-constants
		LUID luidPrivilege;
		// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-lookupprivilegevaluew
		bool succeeded = LookupPrivilegeValueW(nullptr, privilege.c_str(), &luidPrivilege);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__ ": LookupPrivilegeValueW() failed");

		// See https://cpp.hotexamples.com/examples/-/-/AdjustTokenPrivileges/cpp-adjusttokenprivileges-function-examples.html
		// and https://stackoverflow.com/questions/9195889/what-is-the-purpose-of-anysize-array-in-winnt-h
		// and https://web.archive.org/web/20120209061713/http://blogs.msdn.com/b/oldnewthing/archive/2004/08/26/220873.aspx
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luidPrivilege;
		tp.Privileges[0].Attributes = enabled ? SE_PRIVILEGE_ENABLED : 0;

		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-adjusttokenprivileges
		succeeded = AdjustTokenPrivileges(
			token,
			false,
			&tp,
			sizeof(TOKEN_PRIVILEGES),
			nullptr,
			nullptr
		);
		if (succeeded == false)
			throw Error::Win32Error(
				__FUNCSIG__ ": AdjustTokenPrivileges() failed",
				GetLastError()
			);
		if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
			throw Error::Win32Error(
				__FUNCSIG__ ": AdjustTokenPrivileges() could not adjust all privileges",
				GetLastError()
			);
	}

	void SetIntegrity(
		HANDLE token,
		const Constants::GroupIntegrity integrity
	)
	{
		if (token == nullptr || token == INVALID_HANDLE_VALUE)
			throw std::invalid_argument(__FUNCSIG__ ": token cannot be null");

		const std::wstring& integritySid = Constants::Integrities.at(integrity);
		PSID pIntegritySid = nullptr;
		// https://docs.microsoft.com/en-us/windows/win32/api/sddl/nf-sddl-convertstringsidtosidw
		bool succeeded = ConvertStringSidToSidW(integritySid.c_str(), &pIntegritySid);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__": ConvertStringSidToSidW() failed", GetLastError());

		TOKEN_MANDATORY_LABEL tml = { 0 };
		tml.Label.Attributes = SE_GROUP_INTEGRITY;
		tml.Label.Sid = pIntegritySid;
		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-settokeninformation
		succeeded = SetTokenInformation(
			token,
			TokenIntegrityLevel,
			&tml,
			sizeof(TOKEN_MANDATORY_LABEL) + GetLengthSid(pIntegritySid)
		);
		LocalFree(pIntegritySid);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__": SetTokenInformation() failed", GetLastError());
	}
}