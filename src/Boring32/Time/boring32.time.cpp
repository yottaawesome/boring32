module;

#include <sstream>
#include <format>
#include <source_location>
#include "Windows.h"

module boring32.time;
import boring32.error;

namespace Boring32::Time
{
    DWORD SystemTimeToShortIsoDate(const SYSTEMTIME& st)
    {
        return std::stoul(std::format(L"{}{}{}", st.wYear, st.wMonth, st.wDay));
    }

    DWORD SystemTimeToShortIsoDate()
    {
        SYSTEMTIME st;
        GetSystemTime(&st);
        return SystemTimeToShortIsoDate(st);
    }

    uint64_t FromFileTime(const FILETIME& ft)
    {
        const ULARGE_INTEGER uli = { 
            .LowPart = ft.dwLowDateTime,
            .HighPart = ft.dwHighDateTime
        };
        return uli.QuadPart;
    }

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
            throw Error::Win32Error(std::source_location::current(), "FileTimeToSystemTime() failed", lastError);
        }
        return st;
	}

    std::wstring GetTimeAsUtcString(const SYSTEMTIME& st)
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
            throw Error::Win32Error(std::source_location::current(), "GetDateFormatEx() failed", lastError);
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
            throw Error::Win32Error(std::source_location::current(), "GetTimeFormatEx() failed", lastError);
        }

        TIME_ZONE_INFORMATION tzi;
        // https://docs.microsoft.com/en-us/windows/win32/api/datetimeapi/nf-datetimeapi-gettimeformatex
        const DWORD tziStatus = GetTimeZoneInformation(&tzi);
        if(tziStatus == TIME_ZONE_ID_INVALID)
        {
            const auto lastError = GetLastError();
            throw Error::Win32Error(std::source_location::current(), "GetTimeZoneInformation() failed", lastError);
        }

        const long actualBias = tzi.Bias * -1; // should we do this?
        return std::vformat(L"{}-{}.{}{:+}", std::make_wformat_args(dateString, timeString, st.wMilliseconds, actualBias));
    }
}
