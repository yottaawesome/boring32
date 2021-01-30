#pragma once
#include <string>
#include <vector>

namespace Boring32::Strings
{
	std::string ConvertWStringToString(const std::wstring& wstr);
	std::wstring ConvertStringToWString(const std::string& str);
	std::vector<std::wstring> TokeniseString(const std::wstring& stringToTokenise, const std::wstring& delimiter);
	std::wstring Replace(std::wstring source, const std::wstring& from, const std::wstring& to);
	std::wstring Erase(std::wstring source, const wchar_t what);
}
