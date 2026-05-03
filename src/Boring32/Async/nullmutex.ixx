export module boring32:async.nullmutex;

export namespace Boring32::Async
{
	// Conforms to Lockable named requirement.
	// https://en.cppreference.com/w/cpp/named_req/Lockable.html
	struct NullMutex final
	{
		NullMutex(const NullMutex&) = delete;
		auto operator=(const NullMutex&) -> NullMutex& = delete;
		NullMutex(NullMutex&&) = delete;
		auto operator=(NullMutex&&) -> NullMutex& = delete;
		constexpr auto lock() noexcept -> void {}
		constexpr auto unlock() noexcept -> void {}
		constexpr auto try_lock() noexcept -> bool { return true; }
	};
}
