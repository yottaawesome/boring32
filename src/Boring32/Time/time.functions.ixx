export module boring32:time.functions;
import std;
import :win32;
import :error;

export namespace Boring32::Time
{
	auto LargeIntegerTimeToSystemTime(const Win32::LARGE_INTEGER& li) -> Win32::SYSTEMTIME
	{
		auto ft = Win32::FILETIME{
			.dwLowDateTime = li.LowPart,
			.dwHighDateTime = static_cast<Win32::DWORD>(li.HighPart)
		};
		auto st = Win32::SYSTEMTIME{};
		// https://docs.microsoft.com/en-us/windows/win32/api/timezoneapi/nf-timezoneapi-filetimetosystemtime
		if (not Win32::FileTimeToSystemTime(&ft, &st))
			throw Error::Win32Error{Win32::GetLastError(), "FileTimeToSystemTime() failed"};
		return st;
	}

	auto GetTimeAsUTCString(const SYSTEMTIME& st) -> std::wstring
	{
		// Format date buffer
		auto dateString = std::array<wchar_t, 9>{};
		// https://docs.microsoft.com/en-us/windows/win32/api/datetimeapi/nf-datetimeapi-getdateformatex
		auto status = int{
			Win32::GetDateFormatEx(
				Win32::LocaleNameInvariant,
				0,
				&st,
				L"yyyyMMdd",
				dateString.data(),
				static_cast<int>(dateString.size()),
				nullptr
			)};
		if (not status)
			throw Error::Win32Error{Win32::GetLastError(), "GetDateFormatEx() failed"};

		// Format time buffer
		auto timeString = std::array<wchar_t, 9>{};
		// https://docs.microsoft.com/en-us/windows/win32/api/datetimeapi/nf-datetimeapi-gettimeformatex
		status = Win32::GetTimeFormatEx(
			Win32::LocaleNameInvariant,
			0,
			&st,
			L"HH:mm:ss",
			timeString.data(),
			static_cast<int>(timeString.size())
		);
		if (not status)
			throw Error::Win32Error{Win32::GetLastError(), "GetTimeFormatEx() failed"};

		auto tzi = Win32::TIME_ZONE_INFORMATION{};
		// https://docs.microsoft.com/en-us/windows/win32/api/datetimeapi/nf-datetimeapi-gettimeformatex
		auto tziStatus = Win32::DWORD{ Win32::GetTimeZoneInformation(&tzi) };
		if (tziStatus == Win32::TimeZoneIdInvalid)
			throw Error::Win32Error{Win32::GetLastError(), "GetTimeZoneInformation() failed"};

		auto actualBias = long{ tzi.Bias * -1 }; // should we do this?
		return std::format(L"{}-{}.{}{:+}", dateString, timeString, st.wMilliseconds, actualBias);
	}

	auto FromFileTime(const Win32::FILETIME& ft) -> std::uint64_t
	{
		auto uli = Win32::ULARGE_INTEGER{
			.LowPart = ft.dwLowDateTime,
			.HighPart = ft.dwHighDateTime
		};
		return uli.QuadPart;
	}

	auto SystemTimeToShortISODate(const Win32::SYSTEMTIME& st) -> Win32::DWORD
	{
		return std::stoul(std::format(L"{}{}{}", st.wYear, st.wMonth, st.wDay));
	}

	auto SystemTimeToShortISODate() -> Win32::DWORD
	{
		auto st = Win32::SYSTEMTIME{};
		Win32::GetSystemTime(&st);
		return SystemTimeToShortISODate(st);
	}

	auto GetSystemTimeAsUnixTime() -> size_t
	{
		// Adapted from https://stackoverflow.com/a/46024468
		auto ft = Win32::FILETIME{};
		Win32::GetSystemTimeAsFileTime(&ft); //returns ticks in UTC
		//Copy the low and high parts of FILETIME into a LARGE_INTEGER
		//This is so we can access the full 64-bits as an Int64 without causing an alignment fault
		auto li = Win32::LARGE_INTEGER{};
		li.LowPart = ft.dwLowDateTime;
		li.HighPart = ft.dwHighDateTime;
		//Convert ticks since 1/1/1970 into seconds
		// January 1, 1970 (start of Unix epoch) in "ticks"
		constexpr auto UnixTimeStart = size_t{ 0x019DB1DED53E8000 };
		constexpr auto TicksPerSecond = size_t{ 10000000 }; //a tick is 100ns
		return (li.QuadPart - UnixTimeStart) / TicksPerSecond;
	}

	auto FormatTime(
		const Win32::SYSTEMTIME& time,
		const std::wstring& format,
		const std::wstring& locale = Win32::LocaleNameInvariant,
		const Win32::DWORD flags = 0
	) -> std::wstring
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/datetimeapi/nf-datetimeapi-gettimeformatex
		auto charactersNeeded = int{ 
			Win32::GetTimeFormatEx(
				locale.c_str(),
				flags,
				&time,
				&format[0],
				nullptr,
				0
			)};
		if (not charactersNeeded)
			throw Error::Win32Error{Win32::GetLastError(), "GetTimeFormatEx() failed"};

		auto returnVal = std::wstring(charactersNeeded, '\0');
		charactersNeeded = Win32::GetTimeFormatEx(
			locale.c_str(),
			flags,
			&time,
			&format[0],
			&returnVal[0],
			static_cast<int>(returnVal.size())
		);
		if (not charactersNeeded)
			throw Error::Win32Error{Win32::GetLastError(), "GetTimeFormatEx() failed"};
		if (returnVal.back() == '\0')
			returnVal.pop_back(); // remove null terminator if it exists
		return returnVal;
	}

	// https://docs.microsoft.com/en-us/windows/win32/intl/day--month--year--and-era-format-pictures
	auto FormatDate(
		const Win32::SYSTEMTIME& date,
		const std::wstring& format,
		const std::wstring& locale = Win32::LocaleNameInvariant
	) -> std::wstring
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/datetimeapi/nf-datetimeapi-getdateformatex
		auto charactersRequired = int{ 
			Win32::GetDateFormatEx(
				locale.c_str(),
				0,
				&date,
				format.c_str(),
				nullptr,
				0,
				nullptr
			)};
		if (not charactersRequired)
			throw Error::Win32Error{Win32::GetLastError(), "GetDateFormatEx() failed"};

		auto formattedString = std::wstring(charactersRequired, '\0');
		charactersRequired = Win32::GetDateFormatEx(
			locale.c_str(),
			0,
			&date,
			format.c_str(),
			&formattedString[0],
			static_cast<int>(formattedString.size()),
			nullptr
		);
		if (not charactersRequired)
			throw Error::Win32Error{Win32::GetLastError(), "GetDateFormatEx() failed"};
		if (formattedString.back() == '\0')
			formattedString.pop_back(); // remove null terminator if it exists
		return formattedString;
	}

	auto FormatDate(
		const Win32::SYSTEMTIME& date,
		const Win32::DWORD flags,
		const std::wstring& locale = Win32::LocaleNameInvariant
	) -> std::wstring
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/datetimeapi/nf-datetimeapi-getdateformatex
		auto charactersRequired = int{
			Win32::GetDateFormatEx(
				locale.c_str(),
				flags,
				&date,
				nullptr,
				nullptr,
				0,
				nullptr
			)};
		if (not charactersRequired)
			throw Error::Win32Error{Win32::GetLastError(), "GetDateFormatEx() failed"};

		auto formattedString = std::wstring(charactersRequired, '\0');
		charactersRequired = Win32::GetDateFormatEx(
			locale.c_str(),
			flags,
			&date,
			nullptr,
			&formattedString[0],
			static_cast<int>(formattedString.size()),
			nullptr
		);
		if (not charactersRequired)
			throw Error::Win32Error{Win32::GetLastError(), "GetDateFormatEx() failed"};

		return formattedString.c_str();
	}
}