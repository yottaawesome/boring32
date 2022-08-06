module;

#include <cstdint>
#include <Windows.h>

export module boring32.time:datetime;

export namespace Boring32::Time
{
	class DateTime
	{
		public:
			virtual ~DateTime();
			DateTime();
			DateTime(const DateTime& dateTime) = default;
			DateTime(DateTime&& dateTime) noexcept = default;
			DateTime(const SYSTEMTIME& st);
			DateTime(const FILETIME& ft);

		public:
			virtual uint64_t ToMicroSeconds() const noexcept;
			virtual uint64_t ToNanosecondTicks() const noexcept;
			virtual void AddSeconds(const int64_t seconds);
			virtual SYSTEMTIME ToSystemTime() const;

		protected:
			FILETIME m_ft{ 0 };
	};
}
