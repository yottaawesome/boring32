module;

#include <vector>
#include <Windows.h>

export module boring32.async:syncedcontainer;

export namespace Boring32::Async
{
	template<typename T>
	class SyncedContainer
	{
		public:
			virtual ~SyncedContainer()
			{
				DeleteCriticalSection(&m_cs);
			}

			SyncedContainer()
				requires std::is_trivially_constructible<T>::value
			{
				InitializeCriticalSection(&m_cs);
			}

			template<typename...Args>
			SyncedContainer(Args... args)
				: m_protected(args...)
			{
				InitializeCriticalSection(&m_cs);
			}

		public:
			virtual void PopBack()
			{
				m_protected.pop_back();
			}

			virtual void PushBack(typename T::const_reference newValue)
			{
				m_protected.push_back(newValue);
			}

			virtual void Clear()
			{
				m_protected.clear();
			}

		protected:
			T m_protected;
			CRITICAL_SECTION m_cs;
	};
}