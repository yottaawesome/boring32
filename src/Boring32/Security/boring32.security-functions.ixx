export module boring32.security:functions;
import std;

import <win32.hpp>;
import boring32.raii;
import boring32.error;
import :constants;

namespace Boring32::Security
{
	inline constexpr bool IsHandleValid(const HANDLE handle) noexcept
	{
		return handle && handle != INVALID_HANDLE_VALUE;
	}
}

export namespace Boring32::Security
{
	RAII::Win32Handle GetProcessToken(
		const HANDLE processHandle,
		const DWORD desiredAccess
	)
	{
		if (!IsHandleValid(processHandle))
			throw Error::Boring32Error("processHandle cannot be null");

		RAII::Win32Handle handle;
		// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-openprocesstoken
		const bool succeeded = OpenProcessToken(
			processHandle,
			desiredAccess, // https://docs.microsoft.com/en-us/windows/win32/secauthz/access-rights-for-access-token-objects
			&handle
		);
		if (!succeeded)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("OpenProcessToken() failed", lastError);
		}

		return handle;
	}

	void AdjustPrivileges(
		const HANDLE token,
		const std::wstring& privilege,
		const bool enabled
	)
	{
		if (privilege.empty())
			throw Error::Boring32Error("Must specify the privilege name");

		// See also: https://docs.microsoft.com/en-us/windows/win32/secauthz/enabling-and-disabling-privileges-in-c--
		if (!IsHandleValid(token))
			throw Error::Boring32Error(": token cannot be null");

		// https://docs.microsoft.com/en-us/windows/win32/secauthz/privilege-constants
		LUID luidPrivilege;
		// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-lookupprivilegevaluew
		bool succeeded = LookupPrivilegeValueW(nullptr, privilege.c_str(), &luidPrivilege);
		if (!succeeded)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("LookupPrivilegeValueW() failed", lastError);
		}

		// See https://cpp.hotexamples.com/examples/-/-/AdjustTokenPrivileges/cpp-adjusttokenprivileges-function-examples.html
		// and https://stackoverflow.com/questions/9195889/what-is-the-purpose-of-anysize-array-in-winnt-h
		// and https://web.archive.org/web/20120209061713/http://blogs.msdn.com/b/oldnewthing/archive/2004/08/26/220873.aspx
		TOKEN_PRIVILEGES tokenPrivileges
		{
			.PrivilegeCount = 1,
			.Privileges = {
				{
					.Luid = luidPrivilege,
					.Attributes = enabled ? static_cast<DWORD>(SE_PRIVILEGE_ENABLED) : 0
				}
			}
		};

		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-adjusttokenprivileges
		succeeded = AdjustTokenPrivileges(
			token,
			false,
			&tokenPrivileges,
			sizeof(TOKEN_PRIVILEGES),
			nullptr,
			nullptr
		);
		const auto lastError = GetLastError();
		if (lastError != ERROR_SUCCESS)
		{
			// We check lastError, because the function can succeed but GetLastError() can return ERROR_NOT_ALL_ASSIGNED
			throw Error::Win32Error("AdjustTokenPrivileges() failed", lastError);
		}
	}

	void SetIntegrity(
		const HANDLE token,
		const Constants::GroupIntegrity integrity
	)
	{
		if (!IsHandleValid(token))
			throw Error::Boring32Error("token cannot be null");

		const std::wstring& integritySidString = Constants::GetIntegrity(integrity);
		PSID rawIntegritySid = nullptr;
		// https://docs.microsoft.com/en-us/windows/win32/api/sddl/nf-sddl-convertstringsidtosidw
		if (!ConvertStringSidToSidW(integritySidString.c_str(), &rawIntegritySid))
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("ConvertStringSidToSidW() failed", lastError);
		}
		RAII::SIDUniquePtr integritySid(rawIntegritySid);

		TOKEN_MANDATORY_LABEL tml = { 
			.Label = { .Sid = integritySid.get(), .Attributes = SE_GROUP_INTEGRITY }
		};
		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-settokeninformation
		const bool succeeded = SetTokenInformation(
			token,
			TokenIntegrityLevel,
			&tml,
			sizeof(TOKEN_MANDATORY_LABEL) + GetLengthSid(tml.Label.Sid)
		);
		if (!succeeded)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("SetTokenInformation() failed", lastError);
		}
	}

	// See https://docs.microsoft.com/en-us/windows/win32/secauthz/searching-for-a-sid-in-an-access-token-in-c--
	// See also https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-checktokenmembership
	bool SearchTokenGroupsForSID(const HANDLE token, const PSID pSID)
	{
		if (!IsHandleValid(token) || !pSID)
			throw Error::Boring32Error("hToken and pSID cannot be nullptr");

		// Call GetTokenInformation to get the buffer size.
		constexpr unsigned MAX_NAME = 256;
		DWORD dwSize = 0;
		if (!GetTokenInformation(token, TokenGroups, nullptr, 0, &dwSize))
			if (DWORD dwResult = GetLastError(); dwResult != ERROR_INSUFFICIENT_BUFFER)
				throw Error::Win32Error("GetTokenInformation() failed", dwResult);

		// Allocate the buffer.
		std::vector<std::byte> groupInfoBytes(dwSize);
		PTOKEN_GROUPS pGroupInfo = reinterpret_cast<PTOKEN_GROUPS>(&groupInfoBytes[0]);

		// Call GetTokenInformation again to get the group information.
		if (!GetTokenInformation(token, TokenGroups, pGroupInfo, dwSize, &dwSize))
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("GetTokenInformation() failed", lastError);
		}

		// Loop through the group SIDs looking for the SID.
		for (unsigned i = 0; i < pGroupInfo->GroupCount; i++)
			if (EqualSid(pSID, pGroupInfo->Groups[i].Sid))
				return true;

		return false;
	}
	
	void EnumerateTokenGroups(const HANDLE token)
	{
		if (!IsHandleValid(token))
			throw Error::Boring32Error("hToken cannot be nullptr");

		// Call GetTokenInformation to get the buffer size.
		constexpr unsigned MAX_NAME = 256;
		DWORD dwSize = 0;
		DWORD dwResult = 0;
		if (!GetTokenInformation(token, TokenGroups, nullptr, 0, &dwSize))
			if (DWORD dwResult = GetLastError(); dwResult != ERROR_INSUFFICIENT_BUFFER)
				throw Error::Win32Error("GetTokenInformation() failed", dwResult);

		// Allocate the buffer.
		std::vector<std::byte> groupInfoBytes(dwSize);
		PTOKEN_GROUPS pGroupInfo = reinterpret_cast<PTOKEN_GROUPS>(&groupInfoBytes[0]);

		// Call GetTokenInformation again to get the group information.
		if (!GetTokenInformation(token, TokenGroups, pGroupInfo, dwSize, &dwSize))
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("GetTokenInformation() failed", lastError);
		}

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
				throw Error::Win32Error("LookupAccountSidW() failed", dwResult);
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
	void EnumerateTokenPrivileges(const HANDLE token)
	{
		if (!IsHandleValid(token))
			throw Error::Boring32Error("hToken cannot be nullptr");

		DWORD bytesNeeded = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-gettokeninformation
		bool succeeded = GetTokenInformation(
			token,
			TokenPrivileges,
			nullptr,
			0,
			&bytesNeeded
		);
		if (const DWORD lastError = GetLastError(); !succeeded && lastError != ERROR_INSUFFICIENT_BUFFER)
			throw Error::Win32Error("GetTokenInformation() [1] failed", lastError);

		std::vector<std::byte> buffer(bytesNeeded);
		succeeded = GetTokenInformation(
			token,
			TokenPrivileges,
			reinterpret_cast<PBYTE>(&buffer[0]),
			bytesNeeded,
			&bytesNeeded
		);
		if (!succeeded)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("GetTokenInformation() failed", lastError);
		}

		TOKEN_PRIVILEGES* pPrivs = reinterpret_cast<TOKEN_PRIVILEGES*>(&buffer[0]);
		for (unsigned i = 0; i < pPrivs->PrivilegeCount; i++)
		{
			DWORD size = 256;
			std::wstring privName(size, '\0');
			// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-lookupprivilegenamew
			if (!LookupPrivilegeNameW(nullptr, &pPrivs->Privileges[i].Luid, &privName[0], &size))
			{
				const auto lastError = GetLastError();
				throw Error::Win32Error("LookupPrivilegeName() failed", lastError);
			}

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

	bool CheckMembership(const HANDLE token, const PSID sidToCheck)
	{
		if (!IsHandleValid(token))
			throw Error::Boring32Error("hToken cannot be nullptr");
		if (!sidToCheck)
			throw Error::Boring32Error("sidToCheck and pSID cannot be nullptr");

		BOOL result = false;
		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-checktokenmembership
		if (!CheckTokenMembership(token, sidToCheck, &result))
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("CheckTokenMembership() failed", lastError);
		}
		return result;
	}

	enum class AdjustPrivilegeType
	{
		Disable = 0,
		Enable = SE_PRIVILEGE_ENABLED,
		Removed = SE_PRIVILEGE_REMOVED
	};
	bool SetPrivilege(
		const HANDLE token,          // access token handle
		const std::wstring& privilegeName,  // name of privilege to enable/disable
		const AdjustPrivilegeType enablePrivilege   // to enable or disable privilege
	)
	{
		if (!IsHandleValid(token))
			throw Error::Boring32Error("hToken cannot be nullptr");

		// See https://docs.microsoft.com/en-us/windows/win32/secauthz/enabling-and-disabling-privileges-in-c--
		LUID luid;
		// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-lookupprivilegevaluew
		bool succeeded = LookupPrivilegeValueW(
			nullptr,				// lookup privilege on local system
			privilegeName.c_str(),  // privilege to lookup 
			&luid					// receives LUID of privilege
		);
		if (!succeeded)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("LookupPrivilegeValue() failed", lastError);
		}

		// Enable or disable the privilege.
		TOKEN_PRIVILEGES tokenPrivileges{
			.PrivilegeCount = 1,
			.Privileges = {
				{
					.Luid = luid,
					.Attributes = static_cast<DWORD>(enablePrivilege)
				}
			}
		};
		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-adjusttokenprivileges
		succeeded = AdjustTokenPrivileges(
			token,
			false,
			&tokenPrivileges,
			sizeof(TOKEN_PRIVILEGES),
			nullptr,
			nullptr
		);
		if (!succeeded)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("AdjustTokenPrivileges() failed", lastError);
		}

		return GetLastError() == ERROR_NOT_ALL_ASSIGNED ? false : true;
	}

	bool SetPrivilege(
		const HANDLE token,          // access token handle
		const std::vector<std::wstring>& privileges,  // name of privilege to enable/disable
		const AdjustPrivilegeType enablePrivilege   // to enable or disable privilege
	) noexcept
	{
		if (!IsHandleValid(token))
			return false;

		bool allOK = true;
		for (const auto& privilege : privileges) try
		{
			SetPrivilege(token, privilege, enablePrivilege);
		}
		catch (const std::exception& ex)
		{
			std::wcerr << "SetPrivilege()" << ": failed on " << privilege << " : " << ex.what();
			allOK = false;
		}

		return allOK;
	}

	// Make sure to check your privilege
	bool CheckTokenPrivileges(
		const HANDLE token, 
		const bool checkAll, 
		const std::vector<LUID_AND_ATTRIBUTES>& privileges
	)
	{
		if (!IsHandleValid(token))
			throw Error::Boring32Error("token cannot be nullptr");

		// See also https://docs.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-privilege_set
		std::vector<std::byte> privilegesBytes(sizeof(PRIVILEGE_SET) + sizeof(LUID_AND_ATTRIBUTES) * privileges.size());
		PRIVILEGE_SET* privs = reinterpret_cast<PRIVILEGE_SET*>(&privilegesBytes[0]);
		privs->PrivilegeCount = static_cast<DWORD>(privileges.size());
		privs->Control = checkAll ? PRIVILEGE_SET_ALL_NECESSARY : 0;
		for (size_t index = 0; index < privileges.size(); index++)
			privs->Privilege[index] = privileges[index];

		BOOL result = false;
		// Check https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-privilegecheck
		// for notes on the behaviour of this function
		const bool succeeded = PrivilegeCheck(
			token,
			privs,
			&result
		);
		if (!succeeded)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("PrivilegeCheck() failed", lastError);
		}

		return result;
	}

	bool IsLocalSystem()
	{
		// Adapted from https://stackoverflow.com/a/4024388/7448661
		RAII::Win32Handle hToken;
		// open process token
		bool success = OpenProcessToken(
			GetCurrentProcess(),
			TOKEN_QUERY,
			&hToken
		);
		if (!success)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("OpenProcessToken() failed", lastError);
		}

		// retrieve user SID
		// where does this come from?
		std::byte bTokenUser[sizeof(TOKEN_USER) + 8 + 4 * SID_MAX_SUB_AUTHORITIES];
		PTOKEN_USER pTokenUser = reinterpret_cast<PTOKEN_USER>(bTokenUser);
		ULONG cbTokenUser;
		success = GetTokenInformation(
			hToken,
			TokenUser,
			pTokenUser,
			sizeof(bTokenUser),
			&cbTokenUser
		);
		if (!success)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("GetTokenInformation() failed", lastError);
		}

		// allocate LocalSystem well-known SID
		PSID pSystemSid;
		SID_IDENTIFIER_AUTHORITY siaNT = SECURITY_NT_AUTHORITY;
		success = AllocateAndInitializeSid(
			&siaNT,
			1,
			SECURITY_LOCAL_SYSTEM_RID,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			&pSystemSid
		);
		if (!success)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("GetTokenInformation() failed", lastError);
		}

		// compare the user SID from the token with the LocalSystem SID
		bool bSystem = EqualSid(pTokenUser->User.Sid, pSystemSid);

		FreeSid(pSystemSid);

		return bSystem;
	}

	struct SIDInfo
	{
		SID_NAME_USE Type = SID_NAME_USE::SidTypeUser;
		std::wstring Name;
	};

	bool LookupSID(PSID sid, SIDInfo& outSidInfo)
	{
		DWORD size = 256;
		std::wstring name(size, '\0');
		SID_NAME_USE nameUse;
		// https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-lookupaccountsidw
		const bool success = LookupAccountSidW(
			nullptr,
			sid,
			&name[0],
			&size,
			nullptr,
			0,
			&nameUse
		);
		if (success)
		{
			outSidInfo.Type = nameUse;
			outSidInfo.Name = name.c_str(); // trims to size
			return true;
		}

		const auto lastError = GetLastError();
		if (lastError == ERROR_NONE_MAPPED)
			return false;

		throw Error::Win32Error("LookupAccountSidW() failed", lastError);
	}
}