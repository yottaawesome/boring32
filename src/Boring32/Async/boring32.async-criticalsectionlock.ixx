export module boring32:async_criticalsectionlock;
import boring32.win32;

export namespace Boring32::Async
{
	class CriticalSectionLock final
	{
		public:
			~CriticalSectionLock()
			{
				Win32::LeaveCriticalSection(&m_criticalSection);
			}

			CriticalSectionLock(Win32::CRITICAL_SECTION& criticalSection)
				: m_criticalSection(criticalSection)
			{
				Win32::EnterCriticalSection(&m_criticalSection);
			}

			// Non-movable and non-copyable
			CriticalSectionLock(const CriticalSectionLock& other) = delete;
			CriticalSectionLock& operator=(const CriticalSectionLock& other) = delete;
			CriticalSectionLock(const CriticalSectionLock&& other) = delete;
			CriticalSectionLock& operator=(const CriticalSectionLock&& other) = delete;

		private:
			Win32::CRITICAL_SECTION& m_criticalSection;
	};
}
