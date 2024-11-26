export module boring32:async_slimreadwritelock;
import boring32.shared;

export namespace Boring32::Async
{
	struct SharedLockScope final
	{
		~SharedLockScope()
		{
			Win32::ReleaseSRWLockShared(&m_srwLock);
		}

		SharedLockScope(Win32::SRWLOCK& srwLock) noexcept
			: m_srwLock(srwLock)
		{
			Win32::AcquireSRWLockShared(&m_srwLock);
		}

		SharedLockScope(const SharedLockScope&) = delete;
		SharedLockScope(SharedLockScope&&) noexcept = delete;
		SharedLockScope operator=(const SharedLockScope&) = delete;
		SharedLockScope operator=(SharedLockScope&&) noexcept = delete;

		private:
		Win32::SRWLOCK& m_srwLock;
	};

	struct ExclusiveLockScope final
	{
		~ExclusiveLockScope()
		{
			Win32::ReleaseSRWLockExclusive(&m_srwLock);
		}

		ExclusiveLockScope(Win32::SRWLOCK& srwLock) noexcept
			: m_srwLock(srwLock)
		{
			Win32::AcquireSRWLockExclusive(&m_srwLock);
		}

		ExclusiveLockScope(const ExclusiveLockScope&) = delete;
		ExclusiveLockScope(ExclusiveLockScope&&) noexcept = delete;
		ExclusiveLockScope operator=(const ExclusiveLockScope&) = delete;
		ExclusiveLockScope operator=(ExclusiveLockScope&&) noexcept = delete;

		private:
		Win32::SRWLOCK& m_srwLock;
	};

	struct SlimReadWriteLock final
	{
		SlimReadWriteLock()
		{
			Win32::InitializeSRWLock(&m_srwLock);
		}

		//https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-initializesrwlock
		// "An SRW lock cannot be moved or copied."
		SlimReadWriteLock(const SlimReadWriteLock&) = delete;
		SlimReadWriteLock(SlimReadWriteLock&&) noexcept = delete;
		SlimReadWriteLock& operator=(const SlimReadWriteLock&) = delete;
		SlimReadWriteLock& operator=(SlimReadWriteLock&&) noexcept = delete;

		bool TryAcquireSharedLock()
		{
			return Win32::TryAcquireSRWLockShared(&m_srwLock);
		}

		bool TryAcquireExclusiveLock()
		{
			const Win32::DWORD currentThreadId = Win32::GetCurrentThreadId();
			if (m_threadOwningExclusiveLock == currentThreadId)
				return true;
			if (Win32::TryAcquireSRWLockExclusive(&m_srwLock))
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
			const Win32::DWORD currentThreadId = Win32::GetCurrentThreadId();
			if (m_threadOwningExclusiveLock != currentThreadId)
			{
				AcquireSRWLockExclusive(&m_srwLock);
				m_threadOwningExclusiveLock = currentThreadId;
			}
		}

		void ReleaseSharedLock()
		{
			Win32::ReleaseSRWLockShared(&m_srwLock);
		}

		void ReleaseExclusiveLock()
		{
			if (m_threadOwningExclusiveLock != Win32::GetCurrentThreadId())
				return;

			Win32::ReleaseSRWLockExclusive(&m_srwLock);
			m_threadOwningExclusiveLock = 0;
		}

		Win32::SRWLOCK& GetLock() noexcept
		{
			return m_srwLock;
		}

		private:
		Win32::SRWLOCK m_srwLock;
		Win32::DWORD m_threadOwningExclusiveLock = 0;
	};

	template<typename TProtected>
	struct SlimRWProtectedObject final
	{
		SlimRWProtectedObject()
			requires std::is_default_constructible_v<TProtected> = default;
		SlimRWProtectedObject(const TProtected& data)
			requires std::is_copy_constructible_v<TProtected>
			: m_data(data)
		{}
		SlimRWProtectedObject(TProtected&& data)
			requires std::is_move_constructible_v<TProtected>
			: m_data(std::move(data))
		{}

		// NB. I tried experimenting turning this class into a functor with overloaded
		// operator() that accepts either a auto(TProtected&) or auto(const TProtected&)
		// lambda in order to choose which lock scope to create, but I found in my 
		// testing the auto(TProtected&) overload is always preferred over the 
		// auto(const TProtected&) overload. Maybe a better option is to have one 
		// operator() and statically inspect the first argument to see if it's a 
		// const TProtected& or TProtected& and dispatch via a constexpr if from there, 
		// but that's something to figure out another day.
		auto Read(const auto& func, auto&&...args) const
			requires std::is_invocable_v<decltype(func), const TProtected&, decltype(args)...>
		{
			SharedLockScope scope(m_lock.GetLock());
			return func(m_data, std::forward<decltype(args)>(args)...);
		}

		auto Mutate(const auto& func, auto&&...args)
			requires std::is_invocable_v<decltype(func), TProtected&, decltype(args)...>
		{
			ExclusiveLockScope scope(m_lock.GetLock());
			return func(m_data, std::forward<decltype(args)>(args)...);
		}

		private:
		TProtected m_data;
		mutable SlimReadWriteLock m_lock;
	};
}
