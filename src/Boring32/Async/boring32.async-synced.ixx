export module boring32.async:synced;
import <functional>;
import boring32.win32;
import :criticalsectionlock;

export namespace Boring32::Async
{
	template<typename T>
	class Synced final
	{
		public:
			~Synced()
			{
				Win32::DeleteCriticalSection(&m_cs);
			}

			Synced(const Synced&) = delete;
			Synced operator=(const Synced&) = delete;

			Synced() requires std::is_trivially_constructible<T>::value
			{ 
				Win32::InitializeCriticalSection(&m_cs);
			}

			template<typename...Args>
			Synced(Args... args)
				: m_protected(args...)
			{
				Win32::InitializeCriticalSection(&m_cs);
			}

		public:
			T operator()() requires (std::is_copy_constructible<T>::value || std::is_copy_assignable<T>::value)
			{
				CriticalSectionLock cs(m_cs);
				return m_protected;
			}

			auto operator()(const auto X)
			{
				CriticalSectionLock cs(m_cs);
				return X(m_protected);
			}

			Synced<T> operator=(const T& other) requires std::is_copy_assignable<T>::value
			{
				CriticalSectionLock cs(m_cs);
				m_protected = other;
				return *this;
			}

			Synced<T> operator=(T&& other) noexcept requires std::is_move_assignable<T>::value
			{
				CriticalSectionLock cs(m_cs);
				m_protected = other;
				return *this;
			}

		private:
			T m_protected;
			Win32::CRITICAL_SECTION m_cs;
	};
}