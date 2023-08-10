module;

#include <stdint.h>

export module boring32.time:functions;
import std;
import <win32.hpp>;
import boring32.error;


export namespace Boring32::Time
{
	SYSTEMTIME LargeIntegerTimeToSystemTime(const LARGE_INTEGER& li)
	{
		const FILETIME ft{
			.dwLowDateTime = li.LowPart,
			.dwHighDateTime = static_cast<DWORD>(li.HighPart)
		};

		SYSTEMTIME st;
		// https://docs.microsoft.com/en-us/windows/win32/api/timezoneapi/nf-timezoneapi-filetimetosystemtime
		if (!FileTimeToSystemTime(&ft, &st))
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("FileTimeToSystemTime() failed", lastError);
		}
		return st;
	}

	std::wstring GetTimeAsUTCString(const SYSTEMTIME& st)
	{
		// Format date buffer
		constexpr UINT dateStringLength = 9;
		wchar_t dateString[dateStringLength];
		// https://docs.microsoft.com/en-us/windows/win32/api/datetimeapi/nf-datetimeapi-getdateformatex
		int status = GetDateFormatEx(
			LOCALE_NAME_INVARIANT,
			0,
			&st,
			L"yyyyMMdd",
			dateString,
			dateStringLength,
			nullptr
		);
		if (!status)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("GetDateFormatEx() failed", lastError);
		}

		// Format time buffer
		constexpr UINT timeStringLength = 9;
		wchar_t timeString[timeStringLength];
		// https://docs.microsoft.com/en-us/windows/win32/api/datetimeapi/nf-datetimeapi-gettimeformatex
		status = GetTimeFormatEx(
			LOCALE_NAME_INVARIANT,
			0,
			&st,
			L"HH:mm:ss",
			timeString,
			timeStringLength
		);
		if (!status)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("GetTimeFormatEx() failed", lastError);
		}

		TIME_ZONE_INFORMATION tzi;
		// https://docs.microsoft.com/en-us/windows/win32/api/datetimeapi/nf-datetimeapi-gettimeformatex
		const DWORD tziStatus = GetTimeZoneInformation(&tzi);
		if (tziStatus == TIME_ZONE_ID_INVALID)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("GetTimeZoneInformation() failed", lastError);
		}

		const long actualBias = tzi.Bias * -1; // should we do this?
		return std::vformat(L"{}-{}.{}{:+}", std::make_wformat_args(dateString, timeString, st.wMilliseconds, actualBias));
	}

	uint64_t FromFileTime(const FILETIME& ft)
	{
		const ULARGE_INTEGER uli = {
			.LowPart = ft.dwLowDateTime,
			.HighPart = ft.dwHighDateTime
		};
		return uli.QuadPart;
	}

	DWORD SystemTimeToShortISODate(const SYSTEMTIME& st)
	{
		return std::stoul(std::format(L"{}{}{}", st.wYear, st.wMonth, st.wDay));
	}

	DWORD SystemTimeToShortISODate()
	{
		SYSTEMTIME st;
		GetSystemTime(&st);
		return SystemTimeToShortISODate(st);
	}

	size_t GetSystemTimeAsUnixTime()
	{
		// Adapted from https://stackoverflow.com/a/46024468
		// January 1, 1970 (start of Unix epoch) in "ticks"
		static constexpr size_t UnixTimeStart = 0x019DB1DED53E8000;
		static constexpr size_t TicksPerSecond = 10000000; //a tick is 100ns

		FILETIME ft;
		GetSystemTimeAsFileTime(&ft); //returns ticks in UTC

		//Copy the low and high parts of FILETIME into a LARGE_INTEGER
		//This is so we can access the full 64-bits as an Int64 without causing an alignment fault
		LARGE_INTEGER li;
		li.LowPart = ft.dwLowDateTime;
		li.HighPart = ft.dwHighDateTime;

		//Convert ticks since 1/1/1970 into seconds
		return (li.QuadPart - UnixTimeStart) / TicksPerSecond;
	}

	std::wstring FormatTime(
		const SYSTEMTIME& time,
		const std::wstring& format,
		const std::wstring& locale = LOCALE_NAME_INVARIANT,
		const DWORD flags = 0
	)
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/datetimeapi/nf-datetimeapi-gettimeformatex
		int charactersNeeded = GetTimeFormatEx(
			locale.c_str(),
			flags,
			&time,
			&format[0],
			nullptr,
			0
		);
		if (!charactersNeeded)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("GetTimeFormatEx() failed", lastError);
		}

		std::wstring returnVal(charactersNeeded, '\0');
		charactersNeeded = GetTimeFormatEx(
			locale.c_str(),
			flags,
			&time,
			&format[0],
			&returnVal[0],
			static_cast<int>(returnVal.size())
		);
		if (!charactersNeeded)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("GetTimeFormatEx() failed", lastError);
		}

		return returnVal.c_str(); // remove any trailing null
	}

	// https://docs.microsoft.com/en-us/windows/win32/intl/day--month--year--and-era-format-pictures
	std::wstring FormatDate(
		const SYSTEMTIME& date,
		const std::wstring& format,
		const std::wstring& locale = LOCALE_NAME_INVARIANT
	)
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/datetimeapi/nf-datetimeapi-getdateformatex
		int charactersRequired = GetDateFormatEx(
			locale.c_str(),
			0,
			&date,
			format.c_str(),
			nullptr,
			0,
			nullptr
		);
		if (!charactersRequired)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("GetDateFormatEx() failed", lastError);
		}

		std::wstring formattedString(charactersRequired, '\0');
		charactersRequired = GetDateFormatEx(
			locale.c_str(),
			0,
			&date,
			format.c_str(),
			&formattedString[0],
			static_cast<int>(formattedString.size()),
			nullptr
		);
		if (!charactersRequired)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("GetDateFormatEx() failed", lastError);
		}

		return formattedString.c_str();
	}

	std::wstring FormatDate(
		const SYSTEMTIME& date,
		const DWORD flags,
		const std::wstring& locale = LOCALE_NAME_INVARIANT
	)
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/datetimeapi/nf-datetimeapi-getdateformatex
		int charactersRequired = GetDateFormatEx(
			locale.c_str(),
			flags,
			&date,
			nullptr,
			nullptr,
			0,
			nullptr
		);
		if (!charactersRequired)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("GetDateFormatEx() failed", lastError);
		}

		std::wstring formattedString(charactersRequired, '\0');
		charactersRequired = GetDateFormatEx(
			locale.c_str(),
			flags,
			&date,
			nullptr,
			&formattedString[0],
			static_cast<int>(formattedString.size()),
			nullptr
		);
		if (!charactersRequired)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("GetDateFormatEx() failed", lastError);
		}

		return formattedString.c_str();
	}
}