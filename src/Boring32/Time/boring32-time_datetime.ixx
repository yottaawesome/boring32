export module boring32:time_datetime;
import <cstdint>;
import boring32.shared;
import :error;
import :time_functions;

export namespace Boring32::Time
{
	struct DateTime final
	{
		DateTime()
		{
			Win32::GetSystemTimeAsFileTime(&m_ft);
		}

		DateTime(const Win32::SYSTEMTIME& st)
		{
			if (!Win32::SystemTimeToFileTime(&st, &m_ft))
				throw Error::Win32Error(Win32::GetLastError(), "SystemTimeToFileTime() failed");
		}

		DateTime(const Win32::FILETIME& ft)
			: m_ft(ft)
		{ }

		uint64_t ToMicroSeconds() const noexcept
		{
			return FromFileTime(m_ft) / 10ull;
		}

		uint64_t ToNanosecondTicks() const noexcept
		{
			return FromFileTime(m_ft);
		}

		void AddMinutes(const int64_t minutes)
		{
			const int64_t nsTicks = minutes * 60ll * 1000ll * 1000ll * 10ll;
			SetNewTotal(ToNanosecondTicks() + nsTicks);
		}

		void AddSeconds(const int64_t seconds)
		{
			const int64_t nsTicks = seconds * 1000ll * 1000ll * 10ll;
			SetNewTotal(ToNanosecondTicks() + nsTicks);
		}

		void AddMillseconds(const int64_t milliseconds)
		{
			const int64_t nsTicks = milliseconds * 1000ll * 10ll;
			SetNewTotal(ToNanosecondTicks() + nsTicks);
		}

		Win32::SYSTEMTIME ToSystemTime() const
		{
			Win32::SYSTEMTIME st;
			// https://docs.microsoft.com/en-us/windows/win32/api/timezoneapi/nf-timezoneapi-filetimetosystemtime
			if (!Win32::FileTimeToSystemTime(&m_ft, &st))
				throw Error::Win32Error(Win32::GetLastError(), "FileTimeToSystemTime() failed");
			return st;
		}

		private:
		void SetNewTotal(const uint64_t newTotal)
		{
			const Win32::LARGE_INTEGER li{
				.QuadPart = static_cast<long long>(newTotal)
			};
			m_ft = {
				.dwLowDateTime = li.LowPart,
				.dwHighDateTime = static_cast<Win32::DWORD>(li.HighPart)
			};
		}

		Win32::FILETIME m_ft{ 0 };
	};
}
