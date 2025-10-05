export module boring32:async_nullmutex;

export namespace Boring32::Async
{
	// Conforms to Lockable named requirement.
	// https://en.cppreference.com/w/cpp/named_req/Lockable.html
	struct NullMutex final
	{
		NullMutex(const NullMutex&) = delete;
		NullMutex& operator=(const NullMutex&) = delete;
		NullMutex(NullMutex&&) = delete;
		NullMutex& operator=(NullMutex&&) = delete;
		constexpr void lock() noexcept {}
		constexpr void unlock() noexcept {}
		constexpr auto try_lock() noexcept -> bool { return true; }
	};
}
