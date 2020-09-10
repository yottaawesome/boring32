#include "pch.hpp"
#include <stdexcept>
#include "include/Error/Error.hpp"
#include "include/Strings/Strings.hpp"

namespace Boring32::Strings
{
	std::string ConvertWStringToString(const std::wstring& wstr)
	{
		if (wstr.empty())
			return "";

		std::string strTo(wstr.size(), '\0');
		// https://docs.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-widechartomultibyte
		DWORD status = WideCharToMultiByte(
			CP_UTF8,			// CodePage
			0,					// dwFlags 
			wstr.c_str(),		// lpWideCharStr
			(int)wstr.size(),	// cchWideChar 
			&strTo[0],			// lpMultiByteStr
			(int)strTo.size(),	// cbMultiByte
			nullptr,			// lpDefaultChar
			nullptr				// lpUsedDefaultChar
		);
		if (status == 0)
			throw Error::Win32Error("ConvertWStringToString(): WideCharToMultiByte() failed", GetLastError());

		return strTo;
	}

	std::wstring ConvertStringToWString(const std::string& str)
	{
		if (str.empty())
			return L"";

		std::wstring wstrTo(str.size(), '\0');
		// https://docs.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-multibytetowidechar
		DWORD status = MultiByteToWideChar(
			CP_UTF8,			// CodePage
			0,					// dwFlags
			&str[0],			// lpMultiByteStr
			(int)str.size(),	// cbMultiByte
			&wstrTo[0],			// lpWideCharStr
			(int)wstrTo.size()	// cchWideChar
		);
		if (status == 0)
			throw Error::Win32Error("ConvertStringToWString(): MultiByteToWideChar() failed", GetLastError());

		return wstrTo;
	}

	// Older implementation below
	std::string ConvertWStringToStringOld(const std::wstring& str)
	{
		if (str.length() == 0)
			return "";

		// string with embedded nulls are problematic due to wcsrtombs_s exiting after encountering a null.
		// Just error out for now, but a more complete conversion (as I would expect from, say, Boost)
		// would account for this use case.
		size_t nullTerminatorPos = std::string::npos;
		for (int i = 0; i < str.length(); i++)
		{
			if (str[i] == L'\0')
			{
				if (i != str.length() - 1)
					throw new std::runtime_error("Unexpected null terminator found in string");
				if (i == str.length() - 1)
					nullTerminatorPos = i;
			}
		}

		size_t charsConverted = 0;
		size_t reservedSize = nullTerminatorPos != std::string::npos
			? str.length()
			: str.length() + 1; // Needs to be one more to account for \0 added by wcsrtombs_s
		std::string dest(reservedSize, 0);
		mbstate_t mbs;
		mbrlen(NULL, 0, &mbs);
		const wchar_t* a = &str[0];
		errno_t error = wcsrtombs_s(&charsConverted, &dest[0], reservedSize, &a, reservedSize, &mbs);
		dest.pop_back(); // Remove unnecessary null character introduced by wcsrtombs_s

		if (error != 0)
			throw new std::runtime_error("Failed to convert wchar to char");

		return dest;
	}

	std::wstring ConvertStringToWStringOld(const std::string& str)
	{
		if (str.length() == 0)
			return L"";
		size_t nullTerminatorPos = std::string::npos;
		for (int i = 0; i < str.length(); i++)
		{
			if (str[i] == '\0')
			{
				if (i != str.length() - 1)
					throw new std::runtime_error("Unexpected null terminator found in string");
				nullTerminatorPos = i;
			}
		}

		size_t charsConverted = 0;
		size_t reservedSize = nullTerminatorPos != std::string::npos
			? str.length()
			: str.length() + 1; // Needs to be one more to account for \0 added by wcsrtombs_s
		std::wstring dest(reservedSize, 0);
		mbstate_t mbs;
		mbrlen(NULL, 0, &mbs);
		const char* a = &str[0];
		errno_t error = mbsrtowcs_s(&charsConverted, &dest[0], reservedSize, &a, reservedSize, &mbs);
		dest.pop_back(); // Remove unnecessary null character introduced by wcsrtombs_s

		if (error != 0)
			throw new std::runtime_error("Failed to convert wchar to char");

		return dest;
	}

	// Version using wcsrtombs -- it is here purely for reference and should not be used due to security issues
	//char* GenericStrings::ConvertWCharToChar(const wchar_t* str)
	//{
	//	// Reference: http://www.cplusplus.com/reference/cwchar/wcsrtombs/
	//	if (str == nullptr)
	//		throw new std::runtime_error("Parameter str cannot be null");

	//	int strLength = sizeof(str);
	//	if (str[strLength - 1] != '\0')
	//		throw std::runtime_error("Parameter str is not null terminated");

	//	const wchar_t* p;
	//	char* buffer = new char[strLength];
	//	int ret;
	//	mbstate_t mbs;
	//	mbrlen(NULL, 0, &mbs);

	//	p = str;
	//	ret = wcsrtombs(buffer, &p, strLength, &mbs);
	//	if (p != nullptr)
	//		throw new std::runtime_error("Failed to fully convert str");

	//	return buffer;
	//}

	std::vector<std::wstring> TokeniseString(const std::wstring& stringToTokenise, const std::wstring& delimiter)
	{
		std::vector<std::wstring> results;
		size_t position = 0;
		std::wstring intermediateString = stringToTokenise;

		// If we don't find it at all, add the whole string
		if (stringToTokenise.find(delimiter, position) == std::string::npos)
		{
			results.push_back(stringToTokenise);
		}
		else
		{
			while ((position = intermediateString.find(delimiter, position)) != std::string::npos)
			{
				// split and add to the results
				std::wstring split = stringToTokenise.substr(0, position);
				results.push_back(split);

				// move up our position
				position += delimiter.length();
				intermediateString = stringToTokenise.substr(position);

				// On the last iteration, enter the remainder
				if (intermediateString.find(delimiter, position) == std::string::npos)
					results.push_back(intermediateString);
			}
		}

		return results;
	}

	// Adapted from https://stackoverflow.com/a/29752943/7448661
	std::wstring Replace(std::wstring source, const std::wstring& from, const std::wstring& to)
	{
		std::wstring newString;
		newString.reserve(source.length());

		std::wstring::size_type lastPos = 0;
		std::wstring::size_type findPos;

		while (std::wstring::npos != (findPos = source.find(from, lastPos)))
		{
			newString.append(source, lastPos, findPos - lastPos);
			newString += to;
			lastPos = findPos + from.length();
		}

		newString += source.substr(lastPos);

		return newString;
	}
}