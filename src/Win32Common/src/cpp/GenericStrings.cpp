#include "../../include/Win32Common.h"

namespace Win32Common
{
	string GenericStrings::ConvertWStringToString(const wstring& str)
	{
		return ConvertWCharToChar(str.c_str(), str.length()+1);
	}

	wstring GenericStrings::ConvertStringToWString(const string& str)
	{
		return ConvertCharToWChar(str.c_str(), str.length() + 1);
	}

	wchar_t* GenericStrings::ConvertCharToWChar(const char* str, const size_t strLength)
	{
		if (str == nullptr)
			throw new std::runtime_error("Parameter str cannot be null");
		if (strLength < 0)
			throw new std::runtime_error("Parameter strLength cannot be negative");
		if (strLength == 0)
			return new wchar_t[1]{ '\0' };
		if (str[strLength - 1] != '\0')
			throw std::runtime_error("Parameter str is not null terminated");

		size_t charactersConverted = 0;
		wchar_t* destination = new wchar_t[strLength];
		mbstate_t mbs;
		mbrlen(NULL, 0, &mbs);

		errno_t error = mbsrtowcs_s(&charactersConverted, destination, strLength, &str, strLength, &mbs);

		if (error != 0)
			throw new std::runtime_error("Failed to convert wchar to char");

		return destination;
	}

	char* GenericStrings::ConvertWCharToChar(const wchar_t* str, const size_t strLength)
	{
		if (str == nullptr)
			throw new std::runtime_error("Parameter str cannot be null");
		if (strLength < 0)
			throw new std::runtime_error("Parameter strLength cannot be negative");
		if (strLength == 0)
			return new char[1] { '\0' };
		if (str[strLength - 1] != L'\0')
			throw std::runtime_error("Parameter str is not null terminated");

		size_t charsConverted = 0;
		char* destBuffer = new char[strLength];
		mbstate_t mbs;
		mbrlen(NULL, 0, &mbs);

		errno_t error = wcsrtombs_s(&charsConverted, destBuffer, strLength, &str, strLength, &mbs);

		if (error != 0)
			throw new std::runtime_error("Failed to convert wchar to char");

		return destBuffer;
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
}