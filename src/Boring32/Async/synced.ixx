export module boring32:async.synced;
import std;
import :async.criticalsection;

export namespace Boring32::Async
{
	template<typename T>
	class Synced final
	{
	public:
		Synced(const Synced&) = delete;
		auto operator=(const Synced&) -> Synced = delete;

		Synced()
			requires std::is_trivially_constructible<T>::value = default;

		template<typename...Args>
		Synced(Args... args)
			: m_protected(args...)
		{ }

		[[nodiscard]]
		auto operator()() -> T // Do we want this?
			requires std::copyable<T>
		{
			auto cs = CriticalSectionLock(m_cs);
			return m_protected;
		}

		auto operator()(std::invocable<T&> auto&& fn) -> std::invoke_result_t<decltype(fn), T&>
		{
			auto cs = CriticalSectionLock(m_cs);
			return fn(m_protected);
		}

		auto operator=(const T& other) -> Synced&
			requires std::is_copy_assignable_v<T>
		{
			auto cs = CriticalSectionLock(m_cs);
			m_protected = other;
			return *this;
		}

		auto operator=(T&& other) noexcept -> Synced&
			requires std::is_move_assignable_v<T>
		{
			auto cs = CriticalSectionLock(m_cs);
			m_protected = other;
			return *this;
		}

	private:
		T m_protected;
		CriticalSection m_cs;
	};
}