export module boring32:async.criticalsection;
import std;
import :win32;
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
		auto operator=(const CriticalSection&) -> CriticalSection& = delete;
		CriticalSection(const CriticalSection&&) = delete;
		auto operator=(const CriticalSection&&) -> CriticalSection& = delete;

		CriticalSection(Win32::DWORD spinCount = 0)
		{
			// https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-initializecriticalsectionex
			if (not Win32::InitializeCriticalSectionEx(&m_criticalSection, spinCount, 0))
				throw Error::Win32Error{ Win32::GetLastError(), "InitializeCriticalSectionEx() failed" };
		}

		operator Win32::CRITICAL_SECTION*(this CriticalSection& cs) noexcept
		{
			return &cs.m_criticalSection;
		}

		auto lock(this CriticalSection& cs) noexcept -> void
		{ 
			Win32::EnterCriticalSection(&cs.m_criticalSection); 
		}
		
		auto unlock(this CriticalSection& cs) noexcept -> void
		{ 
			Win32::LeaveCriticalSection(&cs.m_criticalSection); 
		}

	private:
		Win32::CRITICAL_SECTION m_criticalSection;
	};

	using CriticalSectionLock = std::scoped_lock<CriticalSection>;
}
