module;

#include <vector>
#include <functional>
#include <algorithm>
#include <source_location>
#include <Windows.h>

export module boring32.async:syncedcontainer;
import :criticalsectionlock;
import boring32.error;

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

			SyncedContainer<T> operator()(
				const std::function<bool(typename T::const_reference)>& func
			)
			{
				CriticalSectionLock cs(m_cs);
				for (auto& x : m_protected)
					if(!func(x))
						return *this;
				return *this;
			}

			auto operator()(const size_t index, const auto func)
			{
				CriticalSectionLock cs(m_cs);
				if (index >= m_protected.size())
					throw Error::Boring32Error(std::source_location::current(), "Invalid index");
				return func(m_protected[index]);
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

			typename T::value_type operator[](const size_t index)
			requires (
				std::is_copy_constructible<typename T::value_type>::value 
				|| std::is_copy_assignable<typename T::value_type>::value
			)
			{
				CriticalSectionLock cs(m_cs);
				if (index >= m_protected.size())
					throw Error::Boring32Error(std::source_location::current(), "Invalid index");
				return m_protected[index];
			}

		public:
			virtual void PopBack()
			{
				CriticalSectionLock cs(m_cs);
				if (m_protected.empty())
					return;
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

			virtual bool Empty()
			{
				CriticalSectionLock cs(m_cs);
				return m_protected.empty();
			}

			virtual void Delete(const size_t index)
			{
				CriticalSectionLock cs(m_cs);
				if (index >= m_protected.size())
					return;
				m_protected.erase(m_protected.begin() + index);
			}

			typename T::value_type Remove(const size_t index)
			requires (
				std::is_copy_constructible<typename T::value_type>::value
				|| std::is_copy_assignable<typename T::value_type>::value
			)
			{
				CriticalSectionLock cs(m_cs);
				if (index >= m_protected.size())
					throw Error::Boring32Error(std::source_location::current(), "Invalid index");
				auto returnVal = m_protected[index];
				m_protected.erase(m_protected.begin() + index);
				return returnVal;
			}
			
		protected:
			T m_protected;
			CRITICAL_SECTION m_cs;
	};
}