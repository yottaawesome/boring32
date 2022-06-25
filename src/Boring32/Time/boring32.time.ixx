module;

#include <string>
#include <Windows.h>

export module boring32.time;

export namespace Boring32::Time
{
	SYSTEMTIME LargeIntegerTimeToSystemTime(const LARGE_INTEGER& li);
	std::wstring GetTimeAsUtcString(const SYSTEMTIME& st);
	uint64_t FromFileTime(const FILETIME& ft);
	DWORD SystemTimeToShortIsoDate(const SYSTEMTIME& st);
	DWORD SystemTimeToShortIsoDate();
	size_t GetSystemTimeAsUnixTime();
	std::wstring FormatTime(
		const SYSTEMTIME& time, 
		const std::wstring& format,
		const std::wstring& locale = LOCALE_NAME_INVARIANT
	);
}
