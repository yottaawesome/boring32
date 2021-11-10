#pragma once
#include <Windows.h>
#include "Constants.hpp"

import boring32.raii.win32handle;

namespace Boring32::Security
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
}