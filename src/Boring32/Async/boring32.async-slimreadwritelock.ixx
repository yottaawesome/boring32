module;

#include <source_location>;

export module boring32.async:slimreadwritelock;
import <stdexcept>;
import <win32.hpp>;

export namespace Boring32::Async
{
	class SharedLockScope
	{
		public:
			~SharedLockScope()
			{
				ReleaseSRWLockShared(&m_srwLock);
			}

			SharedLockScope(SRWLOCK& srwLock)
				: m_srwLock(srwLock)
			{
				AcquireSRWLockShared(&m_srwLock);
			}

			SharedLockScope(const SharedLockScope&) = delete;
			SharedLockScope(SharedLockScope&&) noexcept = delete;
			SharedLockScope operator=(const SharedLockScope&) = delete;
			SharedLockScope operator=(SharedLockScope&&) noexcept = delete;

		private:
			SRWLOCK& m_srwLock;
	};

	class ExclusiveLockScope
	{
		public:
			~ExclusiveLockScope()
			{
				ReleaseSRWLockExclusive(&m_srwLock);
			}

			ExclusiveLockScope(SRWLOCK& srwLock)
				: m_srwLock(srwLock)
			{
				AcquireSRWLockExclusive(&m_srwLock);
			}

			ExclusiveLockScope(const ExclusiveLockScope&) = delete;
			ExclusiveLockScope(ExclusiveLockScope&&) noexcept = delete;
			ExclusiveLockScope operator=(const ExclusiveLockScope&) = delete;
			ExclusiveLockScope operator=(ExclusiveLockScope&&) noexcept = delete;

		private:
			SRWLOCK& m_srwLock;
	};

	class SlimReadWriteLock final
	{
		public:
			~SlimReadWriteLock() = default;
			SlimReadWriteLock()
			{
				InitializeSRWLock(&m_srwLock);
			}

			//https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-initializesrwlock
			// "An SRW lock cannot be moved or copied."
			SlimReadWriteLock(const SlimReadWriteLock&) = delete;
			SlimReadWriteLock(SlimReadWriteLock&&) noexcept = delete;
			SlimReadWriteLock& operator=(const SlimReadWriteLock&) = delete;
			SlimReadWriteLock& operator=(SlimReadWriteLock&&) noexcept = delete;

		public:
			bool TryAcquireSharedLock()
			{
				return TryAcquireSRWLockShared(&m_srwLock);
			}

			bool TryAcquireExclusiveLock()
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

			void AcquireSharedLock()
			{
				AcquireSRWLockShared(&m_srwLock);
			}

			void AcquireExclusiveLock()
			{
				const DWORD currentThreadId = GetCurrentThreadId();
				if (m_threadOwningExclusiveLock != currentThreadId)
				{
					AcquireSRWLockExclusive(&m_srwLock);
					m_threadOwningExclusiveLock = currentThreadId;
				}
			}

			void ReleaseSharedLock()
			{
				ReleaseSRWLockShared(&m_srwLock);
			}

			void ReleaseExclusiveLock()
			{
				if (m_threadOwningExclusiveLock != GetCurrentThreadId())
					return;

				ReleaseSRWLockExclusive(&m_srwLock);
				m_threadOwningExclusiveLock = 0;
			}

			SRWLOCK& GetLock() noexcept
			{
				return m_srwLock;
			}

		private:
			SRWLOCK m_srwLock;
			DWORD m_threadOwningExclusiveLock = 0;
	};
}
