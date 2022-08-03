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
}
