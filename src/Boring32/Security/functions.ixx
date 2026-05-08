export module boring32:security.functions;
import std;
import :win32;
import :raii;
import :error;
import :security.constants;

namespace Boring32::Security
{
	inline constexpr auto IsHandleValid(const Win32::HANDLE handle) noexcept -> bool
	{
		return handle and handle != Win32::InvalidHandleValue;
	}
}

export namespace Boring32::Security
{
	auto GetProcessToken(Win32::HANDLE processHandle, Win32::DWORD desiredAccess) -> RAII::UniqueHandle
	{
		if (not processHandle)
			throw Error::Boring32Error{"processHandle cannot be null"};

		RAII::UniqueHandle handle;
		// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-openprocesstoken
		auto succeeded = Win32::OpenProcessToken(
			processHandle,
			desiredAccess, // https://docs.microsoft.com/en-us/windows/win32/secauthz/access-rights-for-access-token-objects
			&handle
		);
		if (not succeeded)
			throw Error::Win32Error{Win32::GetLastError(), "OpenProcessToken() failed"};

		return handle;
	}

	void AdjustPrivileges(Win32::HANDLE token, const std::wstring& privilege, bool enabled)
	{
		if (privilege.empty())
			throw Error::Boring32Error{"Must specify the privilege name"};
		// See also: https://docs.microsoft.com/en-us/windows/win32/secauthz/enabling-and-disabling-privileges-in-c--
		if (not IsHandleValid(token))
			throw Error::Boring32Error(": token cannot be null");

		// https://docs.microsoft.com/en-us/windows/win32/secauthz/privilege-constants
		auto luidPrivilege = Win32::LUID{};
		// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-lookupprivilegevaluew
		auto succeeded = Win32::LookupPrivilegeValueW(nullptr, privilege.c_str(), &luidPrivilege);
		if (not succeeded)
			throw Error::Win32Error{Win32::GetLastError(), "LookupPrivilegeValueW() failed"};

		// See https://cpp.hotexamples.com/examples/-/-/AdjustTokenPrivileges/cpp-adjusttokenprivileges-function-examples.html
		// and https://stackoverflow.com/questions/9195889/what-is-the-purpose-of-anysize-array-in-winnt-h
		// and https://web.archive.org/web/20120209061713/http://blogs.msdn.com/b/oldnewthing/archive/2004/08/26/220873.aspx
		auto tokenPrivileges = Win32::TOKEN_PRIVILEGES{
			.PrivilegeCount = 1,
			.Privileges = {
				{
					.Luid = luidPrivilege,
					.Attributes = enabled ? static_cast<Win32::DWORD>(Win32::SePrivilegeEnabled) : 0
				}
			}
		};

		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-adjusttokenprivileges
		succeeded = Win32::AdjustTokenPrivileges(
			token,
			false,
			&tokenPrivileges,
			sizeof(Win32::TOKEN_PRIVILEGES),
			nullptr,
			nullptr
		);
		// We check lastError, because the function can succeed but GetLastError() can return ERROR_NOT_ALL_ASSIGNED
		if (auto lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::Success)
			throw Error::Win32Error{lastError, "AdjustTokenPrivileges() failed"};
	}

	void SetIntegrity(Win32::HANDLE token, const Constants::GroupIntegrity integrity)
	{
		if (not IsHandleValid(token))
			throw Error::Boring32Error{"token cannot be null"};

		auto integritySidString = std::wstring_view{Constants::GetIntegrity(integrity)};
		auto rawIntegritySid = Win32::PSID{};
		// https://docs.microsoft.com/en-us/windows/win32/api/sddl/nf-sddl-convertstringsidtosidw
		if (not Win32::ConvertStringSidToSidW(integritySidString.data(), &rawIntegritySid))
			throw Error::Win32Error{Win32::GetLastError(), "ConvertStringSidToSidW() failed"};
		auto integritySid = RAII::SIDUniquePtr{ rawIntegritySid };

		auto tml = Win32::TOKEN_MANDATORY_LABEL{
			.Label = { 
				.Sid = integritySid.get(), 
				.Attributes = Win32::SeGroupIntegrity 
			}
		};
		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-settokeninformation
		auto succeeded = 
			Win32::SetTokenInformation(
				token,
				Win32::TOKEN_INFORMATION_CLASS::TokenIntegrityLevel,
				&tml,
				sizeof(Win32::TOKEN_MANDATORY_LABEL) + Win32::GetLengthSid(tml.Label.Sid)
			);
		if (not succeeded)
			throw Error::Win32Error{Win32::GetLastError(), "SetTokenInformation() failed"};
	}

	// See https://docs.microsoft.com/en-us/windows/win32/secauthz/searching-for-a-sid-in-an-access-token-in-c--
	// See also https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-checktokenmembership
	auto SearchTokenGroupsForSID(Win32::HANDLE token, const Win32::PSID pSID) -> bool
	{
		if (not IsHandleValid(token) or not pSID)
			throw Error::Boring32Error{ "hToken and pSID cannot be nullptr" };

		// Call GetTokenInformation to get the buffer size.
		constexpr auto MAX_NAME = 256u;
		auto dwSize = 0ul;
		if (not Win32::GetTokenInformation(token, Win32::TOKEN_INFORMATION_CLASS::TokenGroups, nullptr, 0, &dwSize))
			if (Win32::DWORD dwResult = Win32::GetLastError(); dwResult != Win32::ErrorCodes::InsufficientBuffer)
				throw Error::Win32Error{dwResult, "GetTokenInformation() failed"};

		// Allocate the buffer.
		auto groupInfoBytes = std::vector<std::byte>(dwSize);
		auto pGroupInfo = reinterpret_cast<Win32::PTOKEN_GROUPS>(&groupInfoBytes[0]);

		// Call GetTokenInformation again to get the group information.
		if (not Win32::GetTokenInformation(token, Win32::TOKEN_INFORMATION_CLASS::TokenGroups, pGroupInfo, dwSize, &dwSize))
			throw Error::Win32Error{Win32::GetLastError(), "GetTokenInformation() failed"};

		// Loop through the group SIDs looking for the SID.
		for (unsigned i = 0; i < pGroupInfo->GroupCount; i++)
			if (Win32::EqualSid(pSID, pGroupInfo->Groups[i].Sid))
				return true;

		return false;
	}
	
	void EnumerateTokenGroups(const Win32::HANDLE token)
	{
		if (not IsHandleValid(token))
			throw Error::Boring32Error("hToken cannot be nullptr");

		// Call GetTokenInformation to get the buffer size.
		constexpr auto MAX_NAME = 256u;
		auto dwSize = Win32::DWORD{};
		auto dwResult = Win32::DWORD{};
		if (not Win32::GetTokenInformation(token, Win32::TOKEN_INFORMATION_CLASS::TokenGroups, nullptr, 0, &dwSize))
			if (auto dwResult = Win32::GetLastError(); dwResult != Win32::ErrorCodes::InsufficientBuffer)
				throw Error::Win32Error{dwResult, "GetTokenInformation() failed"};

		// Allocate the buffer.
		auto groupInfoBytes = std::vector<std::byte>(dwSize);
		auto pGroupInfo = reinterpret_cast<Win32::PTOKEN_GROUPS>(&groupInfoBytes[0]);

		// Call GetTokenInformation again to get the group information.
		if (not Win32::GetTokenInformation(token, Win32::TOKEN_INFORMATION_CLASS::TokenGroups, pGroupInfo, dwSize, &dwSize))
			throw Error::Win32Error{Win32::GetLastError(), "GetTokenInformation() failed"};

		// Loop through the group SIDs looking for the administrator SID.
		auto SidType = Win32::SID_NAME_USE{};
		for (unsigned i = 0; i < pGroupInfo->GroupCount; i++)
		{
			dwSize = MAX_NAME;
			auto groupName = std::wstring(MAX_NAME, '\0');
			auto groupDomain = std::wstring(MAX_NAME, '\0');

			// Lookup the account m_name and print it.
			// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-lookupaccountsidw
			if (not Win32::LookupAccountSidW(
				nullptr,
				pGroupInfo->Groups[i].Sid,
				&groupName[0],
				&dwSize,
				&groupDomain[0],
				&dwSize,
				&SidType)
			){
				if (auto dwResult = Win32::GetLastError(); dwResult == Win32::ErrorCodes::NoneMapped)
				{
					std::wcout << "NONE_MAPPED\n";
					continue;
				}
				throw Error::Win32Error{dwResult, "LookupAccountSidW() failed"};
			}

			groupName = groupName.c_str();
			groupDomain = groupDomain.c_str();
			std::wcout << L"Current user is a member of the " << groupDomain << "\\" << groupName << std::endl;

			// Find out whether the SID is enabled in the token.
			if (pGroupInfo->Groups[i].Attributes & Win32::SeGroupEnabled)
				std::wcout << "The group SID is enabled.\n";
			else if (pGroupInfo->Groups[i].Attributes & Win32::SeGroupUseForDenyOnly)
				std::wcout << "The group SID is a deny-only SID.\n";
			else
				std::wcout << "The group SID is not enabled.\n";
		}
	}

	// See https://docs.microsoft.com/en-us/windows/win32/secauthz/privilege-constants
	// https://docs.microsoft.com/en-us/windows/win32/services/service-user-accounts
	// https://docs.microsoft.com/en-us/windows/security/threat-protection/security-policy-settings/user-rights-assignment
	void EnumerateTokenPrivileges(const Win32::HANDLE token)
	{
		if (not IsHandleValid(token))
			throw Error::Boring32Error("hToken cannot be nullptr");

		auto bytesNeeded = Win32::DWORD{};
		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-gettokeninformation
		auto succeeded = Win32::GetTokenInformation(
			token,
			Win32::TOKEN_INFORMATION_CLASS::TokenPrivileges,
			nullptr,
			0,
			&bytesNeeded
		);
		if (auto lastError = Win32::GetLastError(); !succeeded && lastError != Win32::ErrorCodes::InsufficientBuffer)
			throw Error::Win32Error{lastError, "GetTokenInformation() [1] failed"};

		auto buffer = std::vector<std::byte>(bytesNeeded);
		succeeded = 
			Win32::GetTokenInformation(
				token,
				Win32::TOKEN_INFORMATION_CLASS::TokenPrivileges,
				reinterpret_cast<Win32::PBYTE>(&buffer[0]),
				bytesNeeded,
				&bytesNeeded
			);
		if (not succeeded)
			throw Error::Win32Error{Win32::GetLastError(), "GetTokenInformation() failed"};

		auto pPrivs = reinterpret_cast<Win32::TOKEN_PRIVILEGES*>(&buffer[0]);
		for (unsigned i = 0; i < pPrivs->PrivilegeCount; i++)
		{
			auto size = Win32::DWORD{ 256 };
			auto privName = std::wstring(size, '\0');
			// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-lookupprivilegenamew
			if (not Win32::LookupPrivilegeNameW(nullptr, &pPrivs->Privileges[i].Luid, &privName[0], &size))
				throw Error::Win32Error{Win32::GetLastError(), "LookupPrivilegeName() failed"};

			auto privsString = std::wstring();
			if (pPrivs->Privileges[i].Attributes & Win32::SePrivilegeEnabled)
				privsString += L"enabled; ";
			if (pPrivs->Privileges[i].Attributes & Win32::SePrivilegeEnabledByDefault)
				privsString += L"enabled by default; ";
			if (pPrivs->Privileges[i].Attributes & Win32::SePrivilegeRemoved)
				privsString += L"removed; ";
			if (pPrivs->Privileges[i].Attributes & Win32::SePrivilegeUsedForAccess)
				privsString += L"used for access";

			if (privsString.empty())
				privsString = L"disabled";
			else while (privsString.back() == ' ' or privsString.back() == ';')
				privsString.pop_back();

			privName = std::format(L"{} ({})", privName.c_str(), privsString);
			std::wcout << privName << std::endl;
		}
	}

	bool CheckMembership(const Win32::HANDLE token, const Win32::PSID sidToCheck)
	{
		if (not IsHandleValid(token))
			throw Error::Boring32Error("hToken cannot be nullptr");
		if (not sidToCheck)
			throw Error::Boring32Error("sidToCheck and pSID cannot be nullptr");

		auto result = Win32::BOOL{ false };
		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-checktokenmembership
		if (not Win32::CheckTokenMembership(token, sidToCheck, &result))
			throw Error::Win32Error{Win32::GetLastError(), "CheckTokenMembership() failed"};
		return result;
	}

	enum class AdjustPrivilegeType
	{
		Disable = 0,
		Enable = Win32::SePrivilegeEnabled,
		Removed = Win32::SePrivilegeRemoved
	};
	auto SetPrivilege(
		 Win32::HANDLE token,          // access token handle
		const std::wstring& privilegeName,  // name of privilege to enable/disable
		AdjustPrivilegeType enablePrivilege   // to enable or disable privilege
	) -> bool
	{
		if (not IsHandleValid(token))
			throw Error::Boring32Error{"hToken cannot be nullptr"};

		// See https://docs.microsoft.com/en-us/windows/win32/secauthz/enabling-and-disabling-privileges-in-c--
		auto luid = Win32::LUID{};
		// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-lookupprivilegevaluew
		auto succeeded = 
			Win32::LookupPrivilegeValueW(
				nullptr,				// lookup privilege on local system
				privilegeName.c_str(),  // privilege to lookup 
				&luid					// receives LUID of privilege
			);
		if (not succeeded)
			throw Error::Win32Error{Win32::GetLastError(), "LookupPrivilegeValue() failed"};

		// Enable or disable the privilege.
		auto tokenPrivileges = Win32::TOKEN_PRIVILEGES{
			.PrivilegeCount = 1,
			.Privileges = {
				{
					.Luid = luid,
					.Attributes = static_cast<Win32::DWORD>(enablePrivilege)
				}
			}
		};
		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-adjusttokenprivileges
		succeeded = Win32::AdjustTokenPrivileges(
			token,
			false,
			&tokenPrivileges,
			sizeof(Win32::TOKEN_PRIVILEGES),
			nullptr,
			nullptr
		);
		if (not succeeded)
			throw Error::Win32Error{Win32::GetLastError(), "AdjustTokenPrivileges() failed"};

		return Win32::GetLastError() == Win32::ErrorCodes::NotAllAssigned ? false : true;
	}

	auto SetPrivilege(
		Win32::HANDLE token,          // access token handle
		const std::vector<std::wstring>& privileges,  // name of privilege to enable/disable
		AdjustPrivilegeType enablePrivilege   // to enable or disable privilege
	) noexcept -> bool
	{
		if (not IsHandleValid(token))
			return false;

		auto allOK = true;
		for (const auto& privilege : privileges)
		{
			try
			{
				SetPrivilege(token, privilege, enablePrivilege);
			}
			catch (const std::exception& ex)
			{
				std::wcerr << "SetPrivilege()" << ": failed on " << privilege << " : " << ex.what();
				allOK = false;
			}
		}
		return allOK;
	}

	// Make sure to check your privilege
	auto CheckTokenPrivileges(
		Win32::HANDLE token,
		bool checkAll, 
		const std::vector<Win32::LUID_AND_ATTRIBUTES>& privileges
	) -> bool
	{
		if (not IsHandleValid(token))
			throw Error::Boring32Error{"token cannot be nullptr"};

		// See also https://docs.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-privilege_set
		auto privilegesBytes = std::vector<std::byte>(sizeof(Win32::PRIVILEGE_SET) + sizeof(Win32::LUID_AND_ATTRIBUTES) * privileges.size());
		auto privs = reinterpret_cast<Win32::PRIVILEGE_SET*>(&privilegesBytes[0]);
		privs->PrivilegeCount = static_cast<Win32::DWORD>(privileges.size());
		privs->Control = checkAll ? Win32::PrivilegeSetAllNecessary : 0;
		for (size_t index = 0; index < privileges.size(); index++)
			privs->Privilege[index] = privileges[index];

		auto result = Win32::BOOL{};
		// Check https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-privilegecheck
		// for notes on the behaviour of this function
		auto succeeded = Win32::PrivilegeCheck(token, privs, &result);
		if (not succeeded)
			throw Error::Win32Error{Win32::GetLastError(), "PrivilegeCheck() failed"};
		return result;
	}

	auto IsLocalSystem() -> bool
	{
		// Adapted from https://stackoverflow.com/a/4024388/7448661
		auto hToken = RAII::UniqueHandle{};
		// open process token
		auto success = 
			Win32::OpenProcessToken(Win32::GetCurrentProcess(), Win32::TokenQuery, &hToken);
		if (not success)
			throw Error::Win32Error{Win32::GetLastError(), "OpenProcessToken() failed"};

		// retrieve user SID
		// where does this come from?

		auto bTokenUser = std::array<std::byte, sizeof(Win32::TOKEN_USER) + 8 + 4 * Win32::_SID_MAX_SUB_AUTHORITIES>{};
		auto pTokenUser = reinterpret_cast<Win32::PTOKEN_USER>(bTokenUser.data());
		auto cbTokenUser = Win32::ULONG{};
		success = 
			Win32::GetTokenInformation(
				hToken,
				Win32::TOKEN_INFORMATION_CLASS::TokenUser,
				pTokenUser,
				sizeof(bTokenUser),
				&cbTokenUser
			);
		if (not success)
			throw Error::Win32Error{Win32::GetLastError(), "GetTokenInformation() failed"};

		// allocate LocalSystem well-known SID
		auto pSystemSid = Win32::PSID{};
		auto siaNT = Win32::SID_IDENTIFIER_AUTHORITY{ Win32::_SECURITY_NT_AUTHORITY };
		success = 
			Win32::AllocateAndInitializeSid(
				&siaNT,
				1,
				Win32::_SECURITY_LOCAL_SYSTEM_RID,
				0,
				0,
				0,
				0,
				0,
				0,
				0,
				&pSystemSid
			);
		if (not success)
			throw Error::Win32Error{Win32::GetLastError(), "AllocateAndInitializeSid() failed"};

		// compare the user SID from the token with the LocalSystem SID
		auto bSystem = Win32::EqualSid(pTokenUser->User.Sid, pSystemSid);
		Win32::FreeSid(pSystemSid);
		return bSystem;
	}

	struct SIDInfo
	{
		Win32::SID_NAME_USE Type = Win32::SID_NAME_USE::SidTypeUser;
		std::wstring Name;
	};

	auto LookupSID(Win32::PSID sid, SIDInfo& outSidInfo) -> bool
	{
		auto size = Win32::DWORD{ 256 };
		auto name = std::wstring(size, '\0');
		auto nameUse = Win32::SID_NAME_USE{};
		// https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-lookupaccountsidw
		auto success = 
			Win32::LookupAccountSidW(
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

		auto lastError = Win32::GetLastError();
		return lastError == Win32::ErrorCodes::NoneMapped
			? false
			: throw Error::Win32Error{ lastError, "LookupAccountSidW() failed" };
	}
}