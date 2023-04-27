export module boring32.async:criticalsectionlock;
import <win32.hpp>;

export namespace Boring32::Async
{
	class CriticalSectionLock
	{
		public:
			virtual ~CriticalSectionLock()
			{
				LeaveCriticalSection(&m_criticalSection);
			}

			CriticalSectionLock(CRITICAL_SECTION& criticalSection)
				: m_criticalSection(criticalSection)
			{
				EnterCriticalSection(&m_criticalSection);
			}

			// Non-movable and non-copyable
			CriticalSectionLock(const CriticalSectionLock& other) = delete;
			CriticalSectionLock& operator=(const CriticalSectionLock& other) = delete;
			CriticalSectionLock(const CriticalSectionLock&& other) = delete;
			CriticalSectionLock& operator=(const CriticalSectionLock&& other) = delete;

		protected:
			CRITICAL_SECTION& m_criticalSection;
	};
}
