module;

#include <Windows.h>
#include <string>
#include "include/Security/Constants.hpp"

export module boring32.security.functions;
import boring32.raii.win32handle;

export namespace Boring32::Security
{
	Raii::Win32Handle GetProcessToken(
		const HANDLE processHandle,
		const DWORD desiredAccess
	);
	void AdjustPrivileges(
		HANDLE token,
		const std::wstring& privilege,
		const bool enabled
	);
	void SetIntegrity(
		HANDLE token,
		const Constants::GroupIntegrity integrity
	);
	bool SearchTokenGroupsForSID(HANDLE hToken, PSID pSID);
	void EnumerateTokenGroups(HANDLE hToken);
}