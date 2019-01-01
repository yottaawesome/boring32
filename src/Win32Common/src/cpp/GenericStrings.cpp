#include "../../include/Win32Common.h"

namespace Win32Common
{
	string GenericStrings::ConvertWStringToString(const wstring& str)
	{
		if (str.length() == 0)
			return "";

		// string with embedded nulls are problematic due to wcsrtombs_s exiting after encountering a null.
		// Just error out for now, but a more complete conversion (as I would expect from, say, Boost)
		// would account for this use case.
		size_t nullTerminatorPos = string::npos;
		for (int i = 0; i < str.length(); i++)
		{
			if (str[i] == L'\0')
			{
				if (i != str.length() - 1)
					throw new std::runtime_error("Unexpected null terminator found in string");
				if(i == str.length()-1)
					nullTerminatorPos = i;
			}
		}

		size_t charsConverted = 0;
		size_t reservedSize = nullTerminatorPos != string::npos 
			? str.length()
			: str.length() + 1; // Needs to be one more to account for \0 added by wcsrtombs_s
		string dest(reservedSize, 0);
		mbstate_t mbs;
		mbrlen(NULL, 0, &mbs);
		const wchar_t* a = &str[0];
		errno_t error = wcsrtombs_s(&charsConverted, &dest[0], reservedSize, &a, reservedSize, &mbs);
		dest.pop_back(); // Remove unnecessary null character introduced by wcsrtombs_s

		if (error != 0)
			throw new std::runtime_error("Failed to convert wchar to char");

		return dest;
	}

	wstring GenericStrings::ConvertStringToWString(const string& str)
	{
		if (str.length() == 0)
			return L"";
		size_t nullTerminatorPos = string::npos;
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
		size_t reservedSize = nullTerminatorPos != string::npos
			? str.length()
			: str.length() + 1; // Needs to be one more to account for \0 added by wcsrtombs_s
		wstring dest(reservedSize, 0);
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
}