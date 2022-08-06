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

	DateTime::DateTime(const SYSTEMTIME& st)
	{
		if (!SystemTimeToFileTime(&st, &m_ft))
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("SystemTimeToFileTime() failed", lastError);
		}
	}

	DateTime::DateTime(const FILETIME& ft)
		: m_ft(ft)
	{ }

	uint64_t DateTime::ToMicroSeconds() const noexcept
	{
		return FromFileTime(m_ft) / 10ull;
	}
	
	uint64_t DateTime::ToNanosecondTicks() const noexcept
	{
		return FromFileTime(m_ft);
	}

	void DateTime::AddMinutes(const int64_t minutes)
	{
		const int64_t nsTicks = minutes * 60ll * 1000ll * 1000ll * 10ll;
		SetNewTotal(ToNanosecondTicks() + nsTicks);
	}

	void DateTime::AddSeconds(const int64_t seconds)
	{
		const int64_t nsTicks = seconds * 1000ll * 1000ll * 10ll;
		SetNewTotal(ToNanosecondTicks() + nsTicks);
	}
	
	void DateTime::AddMillseconds(const int64_t milliseconds)
	{
		const int64_t nsTicks = milliseconds * 1000ll * 10ll;
		SetNewTotal(ToNanosecondTicks() + nsTicks);
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

	void DateTime::SetNewTotal(const uint64_t newTotal)
	{
		const LARGE_INTEGER li{
			.QuadPart = static_cast<long long>(newTotal)
		};
		m_ft = {
			.dwLowDateTime = li.LowPart,
			.dwHighDateTime = static_cast<DWORD>(li.HighPart)
		};
	}
}
