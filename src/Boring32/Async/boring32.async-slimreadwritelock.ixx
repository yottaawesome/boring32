module;

#include <source_location>;

export module boring32.async:slimreadwritelock;
import <stdexcept>;
import <win32.hpp>;

export namespace Boring32::Async
{
	class SlimReadWriteLock
	{
		public:
			virtual ~SlimReadWriteLock() = default;
			SlimReadWriteLock()
			{
				InitializeSRWLock(&m_srwLock);
			}

			virtual bool TryAcquireSharedLock()
			{
				return TryAcquireSRWLockShared(&m_srwLock);
			}

			virtual bool TryAcquireExclusiveLock()
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

			virtual void AcquireSharedLock()
			{
				AcquireSRWLockShared(&m_srwLock);
			}

			virtual void AcquireExclusiveLock()
			{
				if (const DWORD currentThreadId = GetCurrentThreadId(); m_threadOwningExclusiveLock != currentThreadId)
				{
					AcquireSRWLockExclusive(&m_srwLock);
					m_threadOwningExclusiveLock = currentThreadId;
				}
			}

			virtual void ReleaseSharedLock()
			{
				ReleaseSRWLockShared(&m_srwLock);
			}

			virtual void ReleaseExclusiveLock()
			{
				if (m_threadOwningExclusiveLock != GetCurrentThreadId())
					return;

				ReleaseSRWLockExclusive(&m_srwLock);
				m_threadOwningExclusiveLock = 0;
			}

			//https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-initializesrwlock
			// "An SRW lock cannot be moved or copied."
		public:
			SlimReadWriteLock(const SlimReadWriteLock&) = delete;
			virtual void operator=(const SlimReadWriteLock&) = delete;
			SlimReadWriteLock(SlimReadWriteLock&&) noexcept = delete;
			virtual void operator=(SlimReadWriteLock&&) noexcept = delete;

		protected:
			SRWLOCK m_srwLock;
			DWORD m_threadOwningExclusiveLock = 0;
	};
}
