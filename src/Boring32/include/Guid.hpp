#pragma once
#include <string>
#include <Windows.h>

namespace Win32Utils::Guid
{
	std::wstring GetGuidAsWString(const GUID& guid);
}
