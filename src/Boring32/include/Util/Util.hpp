#pragma once
#include <string>
#include <stdexcept>
#include <Windows.h>

namespace Boring32::Util
{
	std::wstring GetCurrentExecutableDirectory();
	SYSTEMTIME LargeIntegerTimeToSystemTime(const LARGE_INTEGER& li);
	size_t GetUnixTime();
	DWORD GetMillisToMinuteBoundary(const SYSTEMTIME& time, const UINT boundary);
}
