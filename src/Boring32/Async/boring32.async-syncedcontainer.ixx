module;

#include <vector>
#include <algorithm>
#include <Windows.h>

export module boring32.async:syncedcontainer;
import :criticalsectionlock;

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
			auto operator()(const auto func)
			{
				CriticalSectionLock cs(m_cs);
				return func(m_protected);
			}

			SyncedContainer<T> operator=(const T& other)
			requires std::is_copy_assignable<T>::value
			{
				CriticalSectionLock cs(m_cs);
				m_protected = other;
				return *this;
			}

			SyncedContainer<T> operator=(T&& other) noexcept
			requires std::is_move_assignable<T>::value
			{
				CriticalSectionLock cs(m_cs);
				m_protected = other;
				return *this;
			}

		public:
			virtual void PopBack()
			{
				CriticalSectionLock cs(m_cs);
				m_protected.pop_back();
			}

			virtual void PushBack(typename T::const_reference newValue)
			{
				CriticalSectionLock cs(m_cs);
				m_protected.push_back(newValue);
			}

			virtual void Clear()
			{
				CriticalSectionLock cs(m_cs);
				m_protected.clear();
			}

			virtual size_t Size()
			{
				CriticalSectionLock cs(m_cs);
				return m_protected.size();
			}

			virtual void Delete(const size_t index)
			{
				if (index >= m_protected.size())
					return;
				CriticalSectionLock cs(m_cs);
				m_protected.erase(m_protected.begin() + index);
			}

		protected:
			T m_protected;
			CRITICAL_SECTION m_cs;
	};
}