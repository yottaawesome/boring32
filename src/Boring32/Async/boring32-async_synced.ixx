export module boring32:async_synced;
import std;
import :async_criticalsection;

export namespace Boring32::Async
{
	template<typename T>
	struct Synced final
	{

		Synced(const Synced&) = delete;
		Synced operator=(const Synced&) = delete;

		Synced()
			requires std::is_trivially_constructible<T>::value = default;

		template<typename...Args>
		Synced(Args... args)
			: m_protected(args...)
		{
		}

		T operator()() 
			requires (std::is_copy_constructible<T>::value or std::is_copy_assignable<T>::value)
		{
			CriticalSectionLock cs(m_cs);
			return m_protected;
		}

		auto operator()(const auto X)
		{
			CriticalSectionLock cs(m_cs);
			return X(m_protected);
		}

		Synced operator=(const T& other) 
			requires std::is_copy_assignable<T>::value
		{
			CriticalSectionLock cs(m_cs);
			m_protected = other;
			return *this;
		}

		Synced operator=(T&& other) noexcept 
			requires std::is_move_assignable<T>::value
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