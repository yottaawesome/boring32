#pragma once
#include <string>
#include <vector>

namespace Boring32::Strings
{
	std::string ToString(const std::wstring& wstr);
	std::wstring ToWideString(const std::string& str);
	std::vector<std::wstring> TokeniseString(
		const std::wstring& stringToTokenise, 
		const std::wstring& delimiter
	);
	std::wstring Replace(
		std::wstring source, 
		const std::wstring& from, 
		const std::wstring& to
	);
	std::wstring Erase(std::wstring source, const wchar_t what);
	bool DoCaseInsensitiveMatch(std::string str1, std::string str2);
	bool DoCaseInsensitiveMatch(std::wstring str1, std::wstring str2);
}
