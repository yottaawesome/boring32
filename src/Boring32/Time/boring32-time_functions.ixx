export module boring32:time_functions;
import boring32.shared;
import :error;

export namespace Boring32::Time
{
	Win32::SYSTEMTIME LargeIntegerTimeToSystemTime(const Win32::LARGE_INTEGER& li)
	{
		const Win32::FILETIME ft{
			.dwLowDateTime = li.LowPart,
			.dwHighDateTime = static_cast<Win32::DWORD>(li.HighPart)
		};

		Win32::SYSTEMTIME st;
		// https://docs.microsoft.com/en-us/windows/win32/api/timezoneapi/nf-timezoneapi-filetimetosystemtime
		if (!Win32::FileTimeToSystemTime(&ft, &st))
		{
			const auto lastError = Win32::GetLastError();
			throw Error::Win32Error("FileTimeToSystemTime() failed", lastError);
		}
		return st;
	}

	std::wstring GetTimeAsUTCString(const SYSTEMTIME& st)
	{
		// Format date buffer
		constexpr unsigned dateStringLength = 9;
		wchar_t dateString[dateStringLength];
		// https://docs.microsoft.com/en-us/windows/win32/api/datetimeapi/nf-datetimeapi-getdateformatex
		int status = Win32::GetDateFormatEx(
			Win32::LocaleNameInvariant,
			0,
			&st,
			L"yyyyMMdd",
			dateString,
			dateStringLength,
			nullptr
		);
		if (!status)
		{
			const auto lastError = Win32::GetLastError();
			throw Error::Win32Error("GetDateFormatEx() failed", lastError);
		}

		// Format time buffer
		constexpr unsigned timeStringLength = 9;
		wchar_t timeString[timeStringLength];
		// https://docs.microsoft.com/en-us/windows/win32/api/datetimeapi/nf-datetimeapi-gettimeformatex
		status = Win32::GetTimeFormatEx(
			Win32::LocaleNameInvariant,
			0,
			&st,
			L"HH:mm:ss",
			timeString,
			timeStringLength
		);
		if (!status)
		{
			const auto lastError = Win32::GetLastError();
			throw Error::Win32Error("GetTimeFormatEx() failed", lastError);
		}

		Win32::TIME_ZONE_INFORMATION tzi;
		// https://docs.microsoft.com/en-us/windows/win32/api/datetimeapi/nf-datetimeapi-gettimeformatex
		const Win32::DWORD tziStatus = Win32::GetTimeZoneInformation(&tzi);
		if (tziStatus == Win32::TimeZoneIdInvalid)
		{
			const auto lastError = Win32::GetLastError();
			throw Error::Win32Error("GetTimeZoneInformation() failed", lastError);
		}

		const long actualBias = tzi.Bias * -1; // should we do this?
		return std::vformat(L"{}-{}.{}{:+}", std::make_wformat_args(dateString, timeString, st.wMilliseconds, actualBias));
	}

	uint64_t FromFileTime(const Win32::FILETIME& ft)
	{
		const Win32::ULARGE_INTEGER uli = {
			.LowPart = ft.dwLowDateTime,
			.HighPart = ft.dwHighDateTime
		};
		return uli.QuadPart;
	}

	Win32::DWORD SystemTimeToShortISODate(const Win32::SYSTEMTIME& st)
	{
		return std::stoul(std::format(L"{}{}{}", st.wYear, st.wMonth, st.wDay));
	}

	Win32::DWORD SystemTimeToShortISODate()
	{
		Win32::SYSTEMTIME st;
		Win32::GetSystemTime(&st);
		return SystemTimeToShortISODate(st);
	}

	size_t GetSystemTimeAsUnixTime()
	{
		// Adapted from https://stackoverflow.com/a/46024468
		// January 1, 1970 (start of Unix epoch) in "ticks"
		static constexpr size_t UnixTimeStart = 0x019DB1DED53E8000;
		static constexpr size_t TicksPerSecond = 10000000; //a tick is 100ns

		Win32::FILETIME ft;
		Win32::GetSystemTimeAsFileTime(&ft); //returns ticks in UTC

		//Copy the low and high parts of FILETIME into a LARGE_INTEGER
		//This is so we can access the full 64-bits as an Int64 without causing an alignment fault
		Win32::LARGE_INTEGER li;
		li.LowPart = ft.dwLowDateTime;
		li.HighPart = ft.dwHighDateTime;

		//Convert ticks since 1/1/1970 into seconds
		return (li.QuadPart - UnixTimeStart) / TicksPerSecond;
	}

	std::wstring FormatTime(
		const Win32::SYSTEMTIME& time,
		const std::wstring& format,
		const std::wstring& locale = Win32::LocaleNameInvariant,
		const Win32::DWORD flags = 0
	)
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/datetimeapi/nf-datetimeapi-gettimeformatex
		int charactersNeeded = Win32::GetTimeFormatEx(
			locale.c_str(),
			flags,
			&time,
			&format[0],
			nullptr,
			0
		);
		if (!charactersNeeded)
		{
			const auto lastError = Win32::GetLastError();
			throw Error::Win32Error("GetTimeFormatEx() failed", lastError);
		}

		std::wstring returnVal(charactersNeeded, '\0');
		charactersNeeded = Win32::GetTimeFormatEx(
			locale.c_str(),
			flags,
			&time,
			&format[0],
			&returnVal[0],
			static_cast<int>(returnVal.size())
		);
		if (!charactersNeeded)
		{
			const auto lastError = Win32::GetLastError();
			throw Error::Win32Error("GetTimeFormatEx() failed", lastError);
		}

		return returnVal.c_str(); // remove any trailing null
	}

	// https://docs.microsoft.com/en-us/windows/win32/intl/day--month--year--and-era-format-pictures
	std::wstring FormatDate(
		const SYSTEMTIME& date,
		const std::wstring& format,
		const std::wstring& locale = Win32::LocaleNameInvariant
	)
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/datetimeapi/nf-datetimeapi-getdateformatex
		int charactersRequired = Win32::GetDateFormatEx(
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
			const auto lastError = Win32::GetLastError();
			throw Error::Win32Error("GetDateFormatEx() failed", lastError);
		}

		std::wstring formattedString(charactersRequired, '\0');
		charactersRequired = Win32::GetDateFormatEx(
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
			const auto lastError = Win32::GetLastError();
			throw Error::Win32Error("GetDateFormatEx() failed", lastError);
		}

		return formattedString.c_str();
	}

	std::wstring FormatDate(
		const Win32::SYSTEMTIME& date,
		const Win32::DWORD flags,
		const std::wstring& locale = Win32::LocaleNameInvariant
	)
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/datetimeapi/nf-datetimeapi-getdateformatex
		int charactersRequired = Win32::GetDateFormatEx(
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
			const auto lastError = Win32::GetLastError();
			throw Error::Win32Error("GetDateFormatEx() failed", lastError);
		}

		std::wstring formattedString(charactersRequired, '\0');
		charactersRequired = Win32::GetDateFormatEx(
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
			const auto lastError = Win32::GetLastError();
			throw Error::Win32Error("GetDateFormatEx() failed", lastError);
		}

		return formattedString.c_str();
	}
}