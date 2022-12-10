export module boring32.security:functions;
import boring32.raii;
import :constants;
import <string>;
import <vector>;
import <win32.hpp>;

export namespace Boring32::Security
{
	RAII::Win32Handle GetProcessToken(
		const HANDLE processHandle,
		const DWORD desiredAccess
	);
	void AdjustPrivileges(
		const HANDLE token,
		const std::wstring& privilege,
		const bool enabled
	);
	void SetIntegrity(
		const HANDLE token,
		const Constants::GroupIntegrity integrity
	);
	bool SearchTokenGroupsForSID(const HANDLE hToken, const PSID pSID);
	void EnumerateTokenGroups(const HANDLE hToken);
	void EnumerateTokenPrivileges(const HANDLE hToken);
	bool CheckMembership(const HANDLE token, const PSID sidToCheck);

	enum class AdjustPrivilegeType
	{
		Disable = 0,
		Enable = SE_PRIVILEGE_ENABLED,
		Removed = SE_PRIVILEGE_REMOVED
	};
	bool SetPrivilege(
		const HANDLE hToken,          // access token handle
		const std::wstring& privilegeName,  // name of privilege to enable/disable
		const AdjustPrivilegeType enablePrivilege   // to enable or disable privilege
	);
	bool SetPrivilege(
		const HANDLE hToken,          // access token handle
		const std::vector<std::wstring>& privileges,  // name of privilege to enable/disable
		const AdjustPrivilegeType enablePrivilege   // to enable or disable privilege
	) noexcept;

	// Make sure to check your privilege
	bool CheckTokenPrivileges(
		const HANDLE token, 
		const bool checkAll, 
		const std::vector<LUID_AND_ATTRIBUTES>& privileges
	);
	bool IsLocalSystem();
	struct SIDInfo
	{
		SID_NAME_USE Type = SID_NAME_USE::SidTypeUser;
		std::wstring Name;
	};
	bool LookupSID(PSID sid, SIDInfo& outSidInfo);
}