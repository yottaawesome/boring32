module;

#include <cstdint>
#include <source_location>
#include <Windows.h>

module boring32.time:datetime;
import :functions;
import boring32.error;

namespace Boring32::Time
{
	DateTime::~DateTime() {};

	DateTime::DateTime()
	{
		GetSystemTimeAsFileTime(&m_ft);
	}

	uint64_t DateTime::ToMicroSeconds() const noexcept
	{
		return FromFileTime(m_ft) * 10ull;
	}
	
	uint64_t DateTime::To100NanoSecondIntervals() const noexcept
	{
		return FromFileTime(m_ft);
	}

	void DateTime::AddSeconds(const int64_t seconds)
	{
		const int64_t nanoSecond100s = seconds * 1000 * 1000 * 10;
		const uint64_t newTotal = To100NanoSecondIntervals() + nanoSecond100s;
		const LARGE_INTEGER li{
			.QuadPart = static_cast<long long>(newTotal)
		};
		m_ft = {
			.dwLowDateTime = li.LowPart,
			.dwHighDateTime = static_cast<DWORD>(li.HighPart)
		};
	}

	SYSTEMTIME DateTime::ToSystemTime() const
	{
		SYSTEMTIME st;
		// https://docs.microsoft.com/en-us/windows/win32/api/timezoneapi/nf-timezoneapi-filetimetosystemtime
		if (!FileTimeToSystemTime(&m_ft, &st))
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("FileTimeToSystemTime() failed", lastError);
		}
		return st;
	}
}
