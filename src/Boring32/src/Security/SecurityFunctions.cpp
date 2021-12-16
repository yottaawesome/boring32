module;

#include <stdexcept>
#include <string>
#include <vector>
#include <format>
#include <iostream>
#include <Windows.h>
#include <sddl.h>

module boring32.security.functions;
import boring32.error.win32error;
import boring32.raii.uniqueptrs;
import boring32.security.constants;

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

	void AdjustPrivileges(const HANDLE token, const std::wstring& privilege, const bool enabled)
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
		const HANDLE token,
		const Constants::GroupIntegrity integrity
	)
	{
		if (token == nullptr || token == INVALID_HANDLE_VALUE)
			throw std::invalid_argument(__FUNCSIG__ ": token cannot be null");

		const std::wstring& integritySidString = Constants::GetIntegrity(integrity);
		PSID rawIntegritySid = nullptr;
		// https://docs.microsoft.com/en-us/windows/win32/api/sddl/nf-sddl-convertstringsidtosidw
		if (!ConvertStringSidToSidW(integritySidString.c_str(), &rawIntegritySid))
			throw Error::Win32Error(__FUNCSIG__": ConvertStringSidToSidW() failed", GetLastError());
		Raii::SidUniquePtr integritySid(rawIntegritySid);

		TOKEN_MANDATORY_LABEL tml = { 0 };
		tml.Label.Attributes = SE_GROUP_INTEGRITY;
		tml.Label.Sid = integritySid.get();
		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-settokeninformation
		const bool succeeded = SetTokenInformation(
			token,
			TokenIntegrityLevel,
			&tml,
			sizeof(TOKEN_MANDATORY_LABEL) + GetLengthSid(integritySid.get())
		);
		if (!succeeded)
			throw Error::Win32Error(__FUNCSIG__": SetTokenInformation() failed", GetLastError());
	}

	// See https://docs.microsoft.com/en-us/windows/win32/secauthz/searching-for-a-sid-in-an-access-token-in-c--
	// See also https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-checktokenmembership
    bool SearchTokenGroupsForSID(const HANDLE hToken, const PSID pSID)
    {
		if (!hToken || !pSID)
			throw std::invalid_argument(__FUNCSIG__ "hToken and pSID cannot be nullptr");

        // Call GetTokenInformation to get the buffer size.
		constexpr unsigned MAX_NAME = 256;
		DWORD dwSize = 0;
		if (!GetTokenInformation(hToken, TokenGroups, nullptr, 0, &dwSize))
            if (DWORD dwResult = GetLastError(); dwResult != ERROR_INSUFFICIENT_BUFFER) 
                throw Error::Win32Error(__FUNCSIG__ ": GetTokenInformation() failed", dwResult);

        // Allocate the buffer.
		std::vector<std::byte> groupInfoBytes(dwSize);
		PTOKEN_GROUPS pGroupInfo = reinterpret_cast<PTOKEN_GROUPS>(&groupInfoBytes[0]);

        // Call GetTokenInformation again to get the group information.
        if (!GetTokenInformation(hToken, TokenGroups, pGroupInfo, dwSize, &dwSize))
            throw Error::Win32Error(__FUNCSIG__ ": GetTokenInformation() failed", GetLastError());

        // Loop through the group SIDs looking for the SID.
		for (unsigned i = 0; i < pGroupInfo->GroupCount; i++)
			if (EqualSid(pSID, pGroupInfo->Groups[i].Sid))
				return true;

        return false;
    }

	void EnumerateTokenGroups(const HANDLE hToken)
	{
		if (!hToken)
			throw std::invalid_argument(__FUNCSIG__ "hToken and pSID cannot be nullptr");

		// Call GetTokenInformation to get the buffer size.
		constexpr unsigned MAX_NAME = 256;
		DWORD dwSize = 0;
		DWORD dwResult = 0;
		if (!GetTokenInformation(hToken, TokenGroups, nullptr, 0, &dwSize))
			if (DWORD dwResult = GetLastError(); dwResult != ERROR_INSUFFICIENT_BUFFER)
				throw Error::Win32Error(__FUNCSIG__ ": GetTokenInformation() failed", dwResult);

		// Allocate the buffer.
		std::vector<std::byte> groupInfoBytes(dwSize);
		PTOKEN_GROUPS pGroupInfo = reinterpret_cast<PTOKEN_GROUPS>(&groupInfoBytes[0]);

		// Call GetTokenInformation again to get the group information.
		if (!GetTokenInformation(hToken, TokenGroups, pGroupInfo, dwSize, &dwSize))
			throw Error::Win32Error(__FUNCSIG__ ": GetTokenInformation() failed", GetLastError());

		// Loop through the group SIDs looking for the administrator SID.
		SID_NAME_USE SidType;
		for (unsigned i = 0; i < pGroupInfo->GroupCount; i++)
		{
			dwSize = MAX_NAME;
			std::wstring groupName(MAX_NAME, '\0');
			std::wstring groupDomain(MAX_NAME, '\0');

			// Lookup the account m_name and print it.
			// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-lookupaccountsidw
			if (!LookupAccountSidW(
				nullptr,
				pGroupInfo->Groups[i].Sid,
				&groupName[0],
				&dwSize,
				&groupDomain[0],
				&dwSize,
				&SidType))
			{
				if (DWORD dwResult = GetLastError(); dwResult == ERROR_NONE_MAPPED)
				{
					std::wcout << "NONE_MAPPED\n";
					continue;
				}
				throw Error::Win32Error(__FUNCSIG__": LookupAccountSidW() failed");
			}

			groupName = groupName.c_str();
			groupDomain = groupDomain.c_str();
			std::wcout << L"Current user is a member of the " << groupDomain << "\\" << groupName << std::endl;

			// Find out whether the SID is enabled in the token.
			if (pGroupInfo->Groups[i].Attributes & SE_GROUP_ENABLED)
				std::wcout << "The group SID is enabled.\n";
			else if (pGroupInfo->Groups[i].Attributes & SE_GROUP_USE_FOR_DENY_ONLY)
				std::wcout << "The group SID is a deny-only SID.\n";
			else
				std::wcout << "The group SID is not enabled.\n";
		}
	}

	// See https://docs.microsoft.com/en-us/windows/win32/secauthz/privilege-constants
	// https://docs.microsoft.com/en-us/windows/win32/services/service-user-accounts
	// https://docs.microsoft.com/en-us/windows/security/threat-protection/security-policy-settings/user-rights-assignment
	void EnumerateTokenPrivileges(const HANDLE hToken)
	{
		DWORD bytesNeeded = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-gettokeninformation
		bool succeeded = GetTokenInformation(
			hToken,
			TokenPrivileges,
			nullptr,
			0,
			&bytesNeeded
		);
		if (const DWORD lastError = GetLastError(); !succeeded && lastError != ERROR_INSUFFICIENT_BUFFER)
			throw Error::Win32Error(__FUNCSIG__": GetTokenInformation() [1] failed", lastError);

		std::vector<std::byte> buffer(bytesNeeded);
		succeeded = GetTokenInformation(
			hToken,
			TokenPrivileges,
			reinterpret_cast<PBYTE>(&buffer[0]),
			bytesNeeded,
			&bytesNeeded
		);
		if (!succeeded)
			throw Error::Win32Error(__FUNCSIG__": GetTokenInformation() [2] failed", GetLastError());

		TOKEN_PRIVILEGES* pPrivs = reinterpret_cast<TOKEN_PRIVILEGES*>(&buffer[0]);
		for (unsigned i = 0; i < pPrivs->PrivilegeCount; i++)
		{
			DWORD size = 256;
			std::wstring privName(size, '\0');
			// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-lookupprivilegenamew
			if (!LookupPrivilegeNameW(nullptr, &pPrivs->Privileges[i].Luid, &privName[0], &size))
				throw Error::Win32Error(__FUNCSIG__": LookupPrivilegeName() failed", GetLastError());

			std::wstring privsString;
			if (pPrivs->Privileges[i].Attributes & SE_PRIVILEGE_ENABLED)
				privsString += L"enabled; ";
			if (pPrivs->Privileges[i].Attributes & SE_PRIVILEGE_ENABLED_BY_DEFAULT)
				privsString += L"enabled by default; ";
			if (pPrivs->Privileges[i].Attributes & SE_PRIVILEGE_REMOVED)
				privsString += L"removed; ";
			if (pPrivs->Privileges[i].Attributes & SE_PRIVILEGE_USED_FOR_ACCESS)
				privsString += L"used for access";
			
			if (privsString.empty()) 
				privsString = L"disabled";
			else while (privsString.back() == ' ' || privsString.back() == ';') 
				privsString.pop_back();

			privName = std::format(L"{} ({})", privName.c_str(), privsString);
			std::wcout << privName << std::endl;
		}
	}

	bool SetPrivilege(
		const HANDLE hToken,
		const std::wstring& privilegeName,
		const AdjustPrivilegeType enablePrivilege
	)
	{
		// See https://docs.microsoft.com/en-us/windows/win32/secauthz/enabling-and-disabling-privileges-in-c--
		LUID luid;
		// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-lookupprivilegevaluew
		if (!LookupPrivilegeValueW(
			nullptr,				// lookup privilege on local system
			privilegeName.c_str(),  // privilege to lookup 
			&luid)					// receives LUID of privilege
		) throw Error::Win32Error(__FUNCSIG__": LookupPrivilegeValue() failed", GetLastError());

		TOKEN_PRIVILEGES tokenPrivileges{
			.PrivilegeCount = 1,
			.Privileges = {
				{ 
					.Luid = luid, 
					.Attributes = static_cast<DWORD>(enablePrivilege)
				}
			}
		};

		// Enable or disable the privilege.
		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-adjusttokenprivileges
		if (!AdjustTokenPrivileges(
			hToken,
			false,
			&tokenPrivileges,
			sizeof(TOKEN_PRIVILEGES),
			nullptr,
			nullptr)
		) throw Error::Win32Error(__FUNCSIG__": AdjustTokenPrivileges() failed", GetLastError());

		return GetLastError() == ERROR_NOT_ALL_ASSIGNED ? false : true;
	}

	bool SetPrivilege(
		const HANDLE hToken,
		const std::vector<std::wstring>& privileges,
		const AdjustPrivilegeType enablePrivilege
	) noexcept
	{
		bool allOK = true;
		for (const auto& privilege : privileges) try
		{
			SetPrivilege(hToken, privilege, enablePrivilege);
		}
		catch (const std::exception& ex)
		{
			std::wcerr << "SetPrivilege()" << ": failed on " << privilege << " : " << ex.what();
			allOK = false;
		}

		return allOK;
	}
}