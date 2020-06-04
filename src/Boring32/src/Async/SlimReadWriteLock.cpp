#include "pch.hpp"
#include <stdexcept>
#include "include/Async/SlimReadWriteLock.hpp"

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
		DWORD currentThreadId = GetCurrentThreadId();
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
		DWORD currentThreadId = GetCurrentThreadId();
		if (m_threadOwningExclusiveLock != currentThreadId)
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
		DWORD currentThreadId = GetCurrentThreadId();
		if (m_threadOwningExclusiveLock == currentThreadId)
		{
			ReleaseSRWLockExclusive(&m_srwLock);
			m_threadOwningExclusiveLock = 0;
		}
	}
}
