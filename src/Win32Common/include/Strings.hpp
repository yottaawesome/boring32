#pragma once
#include <string>

namespace Win32Utils::Strings
{
	std::string ConvertWStringToString(const std::wstring& wstr);
	std::wstring ConvertStringToWString(const std::string& str);
	std::wstring GetGuidAsWString(const GUID& guid);
}
