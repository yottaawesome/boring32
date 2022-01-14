
module;

#include <stdexcept>
#include <Windows.h>

module boring32.async.slimreadwritelock;

namespace Boring32::Async
{
	SlimReadWriteLock::~SlimReadWriteLock()
	{ }

	SlimReadWriteLock::SlimReadWriteLock()
	:	m_threadOwningExclusiveLock(0)
	{
		InitializeSRWLock(&m_srwLock);
	}

	bool SlimReadWriteLock::TryAcquireSharedLock()
	{
		return TryAcquireSRWLockShared(&m_srwLock);
	}

	bool SlimReadWriteLock::TryAcquireExclusiveLock()
	{
		const DWORD currentThreadId = GetCurrentThreadId();
		if (m_threadOwningExclusiveLock == currentThreadId)
			return true;
		if (TryAcquireSRWLockExclusive(&m_srwLock))
		{
			m_threadOwningExclusiveLock = currentThreadId;
			return true;
		}
		return false;
	}

	void SlimReadWriteLock::AcquireSharedLock()
	{
		AcquireSRWLockShared(&m_srwLock);
	}

	void SlimReadWriteLock::AcquireExclusiveLock()
	{
		if (const DWORD currentThreadId = GetCurrentThreadId(); m_threadOwningExclusiveLock != currentThreadId)
		{
			AcquireSRWLockExclusive(&m_srwLock);
			m_threadOwningExclusiveLock = currentThreadId;
		}
	}

	void SlimReadWriteLock::ReleaseSharedLock()
	{
		ReleaseSRWLockShared(&m_srwLock);
	}

	void SlimReadWriteLock::ReleaseExclusiveLock()
	{
		if (m_threadOwningExclusiveLock != GetCurrentThreadId())
			return;

		ReleaseSRWLockExclusive(&m_srwLock);
		m_threadOwningExclusiveLock = 0;
	}
}
