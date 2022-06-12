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

			~Synced()
			{
				DeleteCriticalSection(&m_cs);
			}

			template<typename...Args>
			Synced(Args... args)
				: m_protected(args...)
			{
				InitializeCriticalSection(&m_cs);
			}

		public:
			/*virtual Synced<T> operator()(const std::function<void(T&)>& func)
			{
				EnterCriticalSection(&m_cs);
				func(m_protected);
				return *this;
			}*/

			virtual Synced<T> operator()(const FncPtr func)
			{
				EnterCriticalSection(&m_cs);
				func(m_protected);
				return *this;
			}

		protected:
			T m_protected;
			CRITICAL_SECTION m_cs;
	};
}