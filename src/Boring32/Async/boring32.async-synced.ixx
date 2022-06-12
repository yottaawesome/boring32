module;

#include <functional>
#include <Windows.h>

export module boring32.async:synced;

export namespace Boring32::Async
{
	template<typename T>
	class Synced
	{
		public:
			using FncPtr = void(*)(T&);
			template<typename X>
			using FncPtr2 = X(*)(T&);


			~Synced()
			{
				DeleteCriticalSection(&m_cs);
			}

			template<typename S = T> requires std::is_trivially_constructible<S>::value
			Synced() 
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
				EnterCriticalSection(&m_cs);
				return m_protected;
			}

			auto operator()(const auto X)
			{
				EnterCriticalSection(&m_cs);
				return X(m_protected);
			}

			Synced<T> operator=(const T& other)
			requires std::is_copy_assignable<T>::value
			{
				EnterCriticalSection(&m_cs);
				m_protected = other;
				return *this;
			}

			Synced<T> operator=(T&& other) noexcept
			requires std::is_move_assignable<T>::value
			{
				EnterCriticalSection(&m_cs);
				m_protected = other;
				return *this;
			}

		protected:
			T m_protected;
			CRITICAL_SECTION m_cs;
	};
}