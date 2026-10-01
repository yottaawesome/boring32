export module boring32:time.datetime;
import std;
import std.compat;
import :win32;
import :error;
import :time.functions;

export namespace Boring32::Time
{
	class DateTime final
	{
	public:
		DateTime()
		{
			Win32::GetSystemTimeAsFileTime(&m_ft);
		}

		DateTime(const Win32::SYSTEMTIME& st)
		{
			if (not Win32::SystemTimeToFileTime(&st, &m_ft))
				throw Error::Win32Error{Win32::GetLastError(), "SystemTimeToFileTime() failed"};
		}

		DateTime(const Win32::FILETIME& ft)
			: m_ft(ft)
		{ }

		auto ToMicroSeconds() const noexcept -> std::uint64_t
		{
			return FromFileTime(m_ft) / 10ull;
		}

		auto ToNanosecondTicks() const noexcept -> std::uint64_t
		{
			return FromFileTime(m_ft);
		}

		void AddMinutes(const std::int64_t minutes)
		{
			const auto nsTicks = minutes * 60ll * 1000ll * 1000ll * 10ll;
			SetNewTotal(ToNanosecondTicks() + nsTicks);
		}

		void AddSeconds(const std::int64_t seconds)
		{
			const auto nsTicks = seconds * 1000ll * 1000ll * 10ll;
			SetNewTotal(ToNanosecondTicks() + nsTicks);
		}

		void AddMillseconds(const std::int64_t milliseconds)
		{
			const auto nsTicks = milliseconds * 1000ll * 10ll;
			SetNewTotal(ToNanosecondTicks() + nsTicks);
		}

		auto ToSystemTime() const -> Win32::SYSTEMTIME
		{
			auto st = Win32::SYSTEMTIME{};
			// https://docs.microsoft.com/en-us/windows/win32/api/timezoneapi/nf-timezoneapi-filetimetosystemtime
			if (not Win32::FileTimeToSystemTime(&m_ft, &st))
				throw Error::Win32Error{Win32::GetLastError(), "FileTimeToSystemTime() failed"};
			return st;
		}

	private:
		void SetNewTotal(const std::uint64_t newTotal)
		{
			const auto li = Win32::LARGE_INTEGER{
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
