module;

#include "pch.hpp"
#include <sstream>

module boring32.time;

namespace Boring32::Time
{
    DWORD SystemTimeToShortIsoDate(const SYSTEMTIME& st)
    {
        std::wstring s;
        s += std::to_wstring(st.wYear);
        s += std::to_wstring(st.wMonth);
        s += std::to_wstring(st.wDay);
        return std::stoul(s);
    }

    DWORD SystemTimeToShortIsoDate()
    {
        SYSTEMTIME st;
        GetSystemTime(&st);
        return SystemTimeToShortIsoDate(st);
    }

    uint64_t FromFileTime(const FILETIME& ft)
    {
        ULARGE_INTEGER uli = { 0 };
        uli.LowPart = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;
        return uli.QuadPart;
    }

	SYSTEMTIME LargeIntegerTimeToSystemTime(const LARGE_INTEGER& li)
	{
        FILETIME ft;
        ft.dwLowDateTime = li.LowPart;
        ft.dwHighDateTime = li.HighPart;
        SYSTEMTIME st;
        FileTimeToSystemTime(&ft, &st);
        return st;
	}

    std::wstring GetTimeAsUtcString(const SYSTEMTIME& st)
    {
        // Format date buffer
        constexpr UINT dateStringLength = 9;
        wchar_t dateString[dateStringLength];
        GetDateFormatEx(
            LOCALE_NAME_INVARIANT,
            0,
            &st,
            L"yyyyMMdd",
            dateString,
            dateStringLength,
            nullptr
        );

        // Format time buffer
        constexpr UINT timeStringLength = 9;
        wchar_t timeString[timeStringLength];
        GetTimeFormatEx(
            LOCALE_NAME_INVARIANT,
            0,
            &st,
            L"HH:mm:ss",
            timeString,
            timeStringLength
        );

        TIME_ZONE_INFORMATION tzi;
        GetTimeZoneInformation(&tzi);
        std::wstringstream wss;
        wss << dateString
            << L"-"
            << timeString
            << L"."
            << st.wMilliseconds;
        DWORD actualBias = tzi.Bias * -1;
        if (actualBias >= 0)
            wss << L"+";
        wss << actualBias;
        return wss.str();
    }
}
