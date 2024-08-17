export module boring32:async_criticalsection;
import boring32.win32;
import :error;

export namespace Boring32::Async
{
	class CriticalSection final
	{
		public:
			~CriticalSection()
			{
				Win32::DeleteCriticalSection(&m_criticalSection);
			}
			// Non-movable and non-copyable, because the underlying
			// primitive is not movable and not copyable.
			CriticalSection(const CriticalSection&) = delete;
			CriticalSection& operator=(const CriticalSection&) = delete;
			CriticalSection(const CriticalSection&&) = delete;
			CriticalSection& operator=(const CriticalSection&&) = delete;

		public:
			CriticalSection(const Win32::DWORD spinCount = 0)
			{
				Initialise(spinCount);
			}

		public:
			Win32::CRITICAL_SECTION& GetRef() noexcept
			{
				return m_criticalSection;
			}

			const Win32::CRITICAL_SECTION& GetRef() const noexcept
			{
				return m_criticalSection;
			}

			Win32::CRITICAL_SECTION* GetPtr() noexcept
			{
				return &m_criticalSection;
			}

			const Win32::CRITICAL_SECTION* GetPtr() const noexcept
			{
				return &m_criticalSection;
			}

			operator CRITICAL_SECTION*() noexcept
			{
				return &m_criticalSection;
			}

		private:
			void Initialise(const Win32::DWORD spinCount)
			{
				// https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-initializecriticalsectionex
				const bool success = Win32::InitializeCriticalSectionEx(
					&m_criticalSection,
					spinCount,
					0
				);
				if (!success)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error(
						"InitializeCriticalSectionEx() failed",
						lastError
					);
				}
			}

		private:
			Win32::CRITICAL_SECTION m_criticalSection;
	};
}
