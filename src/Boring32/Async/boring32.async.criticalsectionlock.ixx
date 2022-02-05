module;

#include <windows.h>

export module boring32.async.criticalsectionlock;

export namespace Boring32::Async
{
	class CriticalSectionLock
	{
		public:
			virtual ~CriticalSectionLock();
			CriticalSectionLock(CRITICAL_SECTION& criticalSection);

			// Non-movable and non-copyable
			CriticalSectionLock(const CriticalSectionLock& other) = delete;
			CriticalSectionLock& operator=(const CriticalSectionLock& other) = delete;
			CriticalSectionLock(const CriticalSectionLock&& other) = delete;
			CriticalSectionLock& operator=(const CriticalSectionLock&& other) = delete;

		protected:
			CRITICAL_SECTION& m_criticalSection;
	};
}
