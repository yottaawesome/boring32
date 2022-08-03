module;

#include <cstdint>
#include <Windows.h>

module boring32.time:datetime;
import :functions;

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
		int64_t nanoSecond100s = seconds * 1000 * 1000 * 10;
		uint64_t newTotal = To100NanoSecondIntervals() + nanoSecond100s;
		LARGE_INTEGER li{
			.QuadPart = static_cast<long long>(newTotal)
		};
		m_ft = {
			.dwLowDateTime = li.LowPart,
			.dwHighDateTime = static_cast<DWORD>(li.HighPart)
		};
	}
}
