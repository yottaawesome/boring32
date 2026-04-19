export module boring32:async.synced;
import std;
import :async.criticalsection;

export namespace Boring32::Async
{
	template<typename T>
	struct Synced final
	{

		Synced(const Synced&) = delete;
		auto operator=(const Synced&) -> Synced = delete;

		Synced()
			requires std::is_trivially_constructible<T>::value = default;

		template<typename...Args>
		Synced(Args... args)
			: m_protected(args...)
		{
		}

		auto operator()() -> T
		{
			CriticalSectionLock cs(m_cs);
			return m_protected;
		}

		auto operator()(const auto X)
		{
			CriticalSectionLock cs(m_cs);
			return X(m_protected);
		}

		auto operator=(const T& other) -> Synced requires std::is_copy_assignable<T>::value
		{
			CriticalSectionLock cs(m_cs);
			m_protected = other;
			return *this;
		}

		auto operator=(T&& other) noexcept -> Synced requires std::is_move_assignable<T>::value
		{
			CriticalSectionLock cs(m_cs);
			m_protected = other;
			return *this;
		}

		private:
		T m_protected;
		CriticalSection m_cs;
	};
}