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

		public:
			virtual uint64_t ToMicroSeconds() const noexcept;

		protected:
			FILETIME m_ft{ 0 };
	};
}
