export module boring32:async.slimreadwritelock;
import std;
import :win32;

export namespace Boring32::Async
{
	template<bool VShared>
	struct SlimLockScope final
	{
		~SlimLockScope()
		{
			if constexpr (VShared)
				Win32::ReleaseSRWLockShared(&m_srwLock);
			else
				Win32::ReleaseSRWLockExclusive(&m_srwLock);
		}

		SlimLockScope(Win32::SRWLOCK& srwLock) noexcept
			: m_srwLock(srwLock)
		{
			if constexpr (VShared)
				Win32::AcquireSRWLockShared(&m_srwLock);
			else
				Win32::AcquireSRWLockExclusive(&m_srwLock);
		}

		SlimLockScope(const SlimLockScope&) = delete;
		SlimLockScope& operator=(const SlimLockScope&) = delete;

	private:
		Win32::SRWLOCK& m_srwLock;
	};

	using SharedLockScope = SlimLockScope<true>;
	using ExclusiveLockScope = SlimLockScope<false>;

	struct SlimReadWriteLock final
	{
		SlimReadWriteLock()
		{
			Win32::InitializeSRWLock(&m_srwLock);
		}

		//https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-initializesrwlock
		// "An SRW lock cannot be moved or copied."
		SlimReadWriteLock(const SlimReadWriteLock&) = delete;
		SlimReadWriteLock& operator=(const SlimReadWriteLock&) = delete;

		auto TryAcquireSharedLock() -> bool
		{
			return Win32::TryAcquireSRWLockShared(&m_srwLock);
		}

		auto TryAcquireExclusiveLock() -> bool
		{
			Win32::DWORD currentThreadId = Win32::GetCurrentThreadId();
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
			Win32::DWORD currentThreadId = Win32::GetCurrentThreadId();
			if (m_threadOwningExclusiveLock != currentThreadId)
			{
				Win32::AcquireSRWLockExclusive(&m_srwLock);
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

		auto GetLock() noexcept -> Win32::SRWLOCK&
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
			requires std::constructible_from<TProtected> = default;
		SlimRWProtectedObject(std::convertible_to<TProtected> auto&& data)
			: m_data(std::forward<decltype(data)>(data))
		{}

		// NB. I tried experimenting turning this class into a functor with overloaded
		// operator() that accepts either a auto(TProtected&) or auto(const TProtected&)
		// lambda in order to choose which lock scope to create, but I found in my 
		// testing the auto(TProtected&) overload is always preferred over the 
		// auto(const TProtected&) overload. Maybe a better option is to have one 
		// operator() and statically inspect the first argument to see if it's a 
		// const TProtected& or TProtected& and dispatch via a constexpr if from there, 
		// but that's something to figure out another day.
		auto Read(this auto&& self, auto&& func, auto&&...args)
			requires std::invocable<decltype(func), const TProtected&, decltype(args)...>
		{
			SharedLockScope scope(self.m_lock.GetLock());
			return std::invoke(self.m_data, std::forward<decltype(args)>(args)...);
		}

		auto Mutate(this auto&& self, auto&& func, auto&&...args)
			requires std::invocable<decltype(func), const TProtected&, decltype(args)...>
		{
			ExclusiveLockScope scope(self.m_lock.GetLock());
			return func(self.m_data, std::forward<decltype(args)>(args)...);
		}

		auto operator=(this auto&& self, std::convertible_to<TProtected> auto&& other) -> SlimRWProtectedObject
		{
			self.Mutate(
				[](auto& current, auto&& other) static { current = std::forward<decltype(other)>(other); }, 
				std::forward<decltype(other)>(other));
			return self;
		}

		operator TProtected(this auto&& self)
			requires std::copy_constructible<TProtected>
		{
			return self.Read([](const auto& current) { return current; });
		}

	private:
		TProtected m_data;
		mutable SlimReadWriteLock m_lock;
	};
}
