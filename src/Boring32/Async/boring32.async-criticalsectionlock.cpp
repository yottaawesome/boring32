module;

#include <Windows.h>

module boring32.async:criticalsectionlock;

namespace Boring32::Async
{
	CriticalSectionLock::CriticalSectionLock(CRITICAL_SECTION& criticalSection)
		: m_criticalSection(criticalSection)
	{
		EnterCriticalSection(&m_criticalSection);
	}

	CriticalSectionLock::~CriticalSectionLock()
	{
		LeaveCriticalSection(&m_criticalSection);
	}
}