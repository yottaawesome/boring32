module;

#include <string>
#include <vector>
#include <Windows.h>

export module boring32.security.functions;
import boring32.raii.win32handle;
import boring32.security.constants;

export namespace Boring32::Security
{
	Raii::Win32Handle GetProcessToken(
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
}