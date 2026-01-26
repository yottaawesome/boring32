export module boring32:async.criticalsection;
import std;
import :win32;
import :error;

export namespace Boring32::Async
{
	struct CriticalSection final
	{
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

		CriticalSection(Win32::DWORD spinCount = 0)
		{
			Initialise(spinCount);
		}

		operator Win32::CRITICAL_SECTION* () noexcept
		{
			return &m_criticalSection;
		}

		void lock() noexcept
		{ 
			Win32::EnterCriticalSection(&m_criticalSection); 
		}
		
		void unlock() noexcept
		{ 
			Win32::LeaveCriticalSection(&m_criticalSection); 
		}

	private:
		void Initialise(Win32::DWORD spinCount)
		{
			// https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-initializecriticalsectionex
			if (not Win32::InitializeCriticalSectionEx(&m_criticalSection, spinCount, 0))
				throw Error::Win32Error(Win32::GetLastError(), "InitializeCriticalSectionEx() failed");
		}

		Win32::CRITICAL_SECTION m_criticalSection;
	};

	using CriticalSectionLock = std::scoped_lock<CriticalSection>;
}
