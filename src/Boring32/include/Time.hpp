#pragma once
#include <string>
#include <Windows.h>

namespace Boring32::Time
{
	SYSTEMTIME LargeIntegerTimeToSystemTime(const LARGE_INTEGER& li);
	std::wstring GetTimeAsUtcString(const SYSTEMTIME& st);
	uint64_t FromFileTime(const FILETIME& ft);
	DWORD SystemTimeToShortIsoDate(const SYSTEMTIME& st);
	DWORD SystemTimeToShortIsoDate();
}
