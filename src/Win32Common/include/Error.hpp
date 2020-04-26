#pragma once
#include <string>
#include <Windows.h>

namespace Win32Utils::Error
{
	std::wstring GetErrorCodeAsWString(const DWORD errorCode);
}
