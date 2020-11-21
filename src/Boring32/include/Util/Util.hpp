#pragma once
#include <string>
#include <stdexcept>
#include <Windows.h>

#define colconc(str1,str2) #str1 ": " #str2

namespace Boring32::Util
{
	std::wstring GetCurrentExecutableDirectory();
	SYSTEMTIME LargeIntegerTimeToSystemTime(const LARGE_INTEGER& li);
	size_t GetUnixTime();
	DWORD GetMillisToMinuteBoundary(const SYSTEMTIME& time, const UINT boundary);
}
