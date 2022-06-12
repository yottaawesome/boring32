module;

#include <functional>
#include <Windows.h>

export module boring32.async:synced;
import :criticalsectionlock;

export namespace Boring32::Async
{
	template<typename T>
	class Synced
	{
		public:
			virtual ~Synced()
			{
				DeleteCriticalSection(&m_cs);
			}

			Synced() 
			requires std::is_trivially_constructible<T>::value
			{ 
				InitializeCriticalSection(&m_cs); 
			}

			template<typename...Args>
			Synced(Args... args)
				: m_protected(args...)
			{
				InitializeCriticalSection(&m_cs);
			}

		public:
			T operator()()
			requires (std::is_copy_constructible<T>::value || std::is_copy_assignable<T>::value)
			{
				CriticalSectionLock(m_cs);
				return m_protected;
			}

			auto operator()(const auto X)
			{
				CriticalSectionLock(m_cs);
				return X(m_protected);
			}

			Synced<T> operator=(const T& other)
			requires std::is_copy_assignable<T>::value
			{
				CriticalSectionLock(m_cs);
				m_protected = other;
				return *this;
			}

			Synced<T> operator=(T&& other) noexcept
			requires std::is_move_assignable<T>::value
			{
				CriticalSectionLock(m_cs);
				m_protected = other;
				return *this;
			}

		protected:
			T m_protected;
			CRITICAL_SECTION m_cs;
	};
}