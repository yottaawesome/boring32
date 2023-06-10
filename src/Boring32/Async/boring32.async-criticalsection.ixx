export module boring32.async:criticalsection;
import boring32.error;
import <win32.hpp>;

export namespace Boring32::Async
{
	class CriticalSection
	{
		public:
			~CriticalSection()
			{
				DeleteCriticalSection(&m_criticalSection);
			}
			CriticalSection() = delete;
			// Non-movable and non-copyable
			CriticalSection(const CriticalSection&) = delete;
			CriticalSection& operator=(const CriticalSection&) = delete;
			CriticalSection(const CriticalSection&&) = delete;
			CriticalSection& operator=(const CriticalSection&&) = delete;

		public:
			CriticalSection(const DWORD spinCount = 0)
			{
				Initialise(spinCount);
			}

		public:
			CRITICAL_SECTION& GetRef() noexcept
			{
				return m_criticalSection;
			}

			const CRITICAL_SECTION& GetRef() const noexcept
			{
				return m_criticalSection;
			}

			CRITICAL_SECTION* GetPtr() noexcept
			{
				return &m_criticalSection;
			}

			const CRITICAL_SECTION* GetPtr() const noexcept
			{
				return &m_criticalSection;
			}

			operator CRITICAL_SECTION*() noexcept
			{
				return &m_criticalSection;
			}

		private:
			void Initialise(const DWORD spinCount)
			{
				// https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-initializecriticalsectionex
				const bool success = InitializeCriticalSectionEx(
					&m_criticalSection,
					spinCount,
					0
				);
				if (!success)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error(
						"InitializeCriticalSectionEx() failed",
						lastError
					);
				}
			}

		private:
			CRITICAL_SECTION m_criticalSection;
	};
}
