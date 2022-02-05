module;

#include <Windows.h>

export module boring32.async.slimreadwritelock;

export namespace Boring32::Async
{
	class SlimReadWriteLock
	{
		public:
			virtual ~SlimReadWriteLock();
			SlimReadWriteLock();

			virtual bool TryAcquireSharedLock();
			virtual bool TryAcquireExclusiveLock();

			virtual void AcquireSharedLock();
			virtual void AcquireExclusiveLock();

			virtual void ReleaseSharedLock();
			virtual void ReleaseExclusiveLock();

			//https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-initializesrwlock
			// "An SRW lock cannot be moved or copied."
		public:
			SlimReadWriteLock(const SlimReadWriteLock&) = delete;
			virtual void operator=(const SlimReadWriteLock&) = delete;
			SlimReadWriteLock(SlimReadWriteLock&&) noexcept = delete;
			virtual void operator=(SlimReadWriteLock&&) noexcept = delete;

		protected:
			SRWLOCK m_srwLock;
			DWORD m_threadOwningExclusiveLock;
	};
}
