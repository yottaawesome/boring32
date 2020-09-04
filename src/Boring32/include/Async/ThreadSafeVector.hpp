#pragma once
#include <functional>
#include <vector>
#include <Windows.h>
#include "Event.hpp"
#include "CriticalSectionLock.hpp"

namespace Boring32::Async
{
	template<typename T>
	class ThreadSafeVector
	{
		public:
			virtual ~ThreadSafeVector()
			{
				DeleteCriticalSection(&m_criticalSection);
			}

			ThreadSafeVector()
				: m_hasMessages(false, true, false, L"")
			{
				InitializeCriticalSection(&m_criticalSection);
			}

			virtual void Add(T&& msg) noexcept
			{
				CriticalSectionLock cs(m_criticalSection);
				m_collection.push_back(msg);
				m_hasMessages.Signal();
			}

			virtual void Add(T& msg)
			{
				CriticalSectionLock cs(m_criticalSection);
				m_collection.push_back(msg);
				m_hasMessages.Signal();
			}

			virtual size_t Size()
			{
				CriticalSectionLock cs(m_criticalSection);
				return m_collection.size();
			}

			virtual std::vector<T> ToVector() const
			{
				return std::vector<T>(m_collection);
			}

			virtual void Clear()
			{
				CriticalSectionLock cs(m_criticalSection);
				m_collection.clear();
				m_hasMessages.Reset();
			}

			virtual void DoWithLock(std::function<void(std::vector<T>&)>& func)
			{
				CriticalSectionLock cs(m_criticalSection);
				func(m_collection);
			}

			virtual void DoWithLock(std::function<void(std::vector<T>&)>&& func)
			{
				CriticalSectionLock cs(m_criticalSection);
				func(m_collection);
			}

			virtual T CopyOfElementAt(const int index)
			{
				CriticalSectionLock cs(m_criticalSection);
				return m_collection.at(index);
			}

			virtual void ForEach(std::function<bool(T&)>& func)
			{
				CriticalSectionLock cs(m_criticalSection);
				for (T& item : m_collection)
					if (func(item) == false)
						break;;
			}

			virtual void ForEach(std::function<bool(T&)>&& func)
			{
				CriticalSectionLock cs(m_criticalSection);
				for (T& item : m_collection)
					if (func(item) == false)
						break;;
			}

			virtual void ForEachAndClear(std::function<void(T&)>& func)
			{
				CriticalSectionLock cs(m_criticalSection);
				for (T& item : m_collection)
					func(item);
				Clear();
			}

			virtual void ForEachAndClear(std::function<void(T&)>&& func)
			{
				CriticalSectionLock cs(m_criticalSection);
				for (T& item : m_collection)
					func(item);
				Clear();
			}

			virtual void ForEachAndClear(std::function<void(T*)>& func)
			{
				CriticalSectionLock cs(m_criticalSection);
				for (T& item : m_collection)
					func(&item);
				Clear();
			}

			virtual void ForEachAndClear(std::function<void(T*)>&& func)
			{
				CriticalSectionLock cs(m_criticalSection);
				for (T& item : m_collection)
					func(&item);
				Clear();
			}

			virtual std::tuple<size_t, size_t> ForEachAndSelectiveClear(std::function<bool(T&)>& func)
			{
				return InternalForEachAndSelectiveClear(func);
			}

			virtual std::tuple<size_t, size_t> ForEachAndSelectiveClear(std::function<bool(T&)>&& func)
			{
				return InternalForEachAndSelectiveClear(func);
			}

			virtual bool EraseOne(const std::function<bool(const T&)>& findFunc)
			{
				CriticalSectionLock cs(m_criticalSection);

				int index = IndexOf(findFunc);
				if (index > -1)
				{
					RemoveAt(index);
					if (m_collection.size() == 0)
						m_hasMessages.Reset();
				}
				return index > -1;
			}

			virtual void EraseMultiple(const std::function<bool(const T&)>& findFunc)
			{
				CriticalSectionLock cs(m_criticalSection);
				int index = 0;
				while ((index = IndexOf(findFunc)) != -1)
				{
					RemoveAt(index);
					if (m_collection.size() == 0)
						m_hasMessages.Reset();
				}
			}

			virtual bool FindAndErase(const std::function<bool(const T&)>& findFunc, T& itemToSet)
			{
				CriticalSectionLock cs(m_criticalSection);
				int index = IndexOf(findFunc);
				if (index > -1)
				{
					itemToSet = m_collection.at(index);
					RemoveAt(index);
					if (m_collection.size() == 0)
						m_hasMessages.Reset();
					return true;
				}
				return false;
			}

			virtual void RemoveAt(const size_t index)
			{
				CriticalSectionLock cs(m_criticalSection);
				if (index < m_collection.size())
				{
					m_collection.erase(m_collection.begin() + index);
					if (m_collection.size() == 0)
						m_hasMessages.Reset();
				}
			}

			virtual int IndexOf(const std::function<bool(const T&)>& func)
			{
				CriticalSectionLock cs(m_criticalSection);
				for (int i = 0; i < m_collection.size(); i++)
				{
					if (func(m_collection[i]))
						return i;
				}
				return -1;
			}

			virtual HANDLE GetWaitableHandle()
			{
				return m_hasMessages.GetHandle();
			}

		protected:
			virtual std::tuple<size_t, size_t> InternalForEachAndSelectiveClear(std::function<bool(T&)>& func)
			{
				CriticalSectionLock cs(m_criticalSection);
				size_t originalCount = m_collection.size();
				std::vector<int> indexesToKeep;
				for (int i = 0; i < m_collection.size(); i++)
				{
					if (func(m_collection[i]))
						indexesToKeep.push_back(i);
				}
				size_t itemCountRemoved = m_collection.size() - indexesToKeep.size();
				std::vector<T> newCollection(indexesToKeep.size());
				for (int index : indexesToKeep)
					newCollection.push_back(m_collection[index]);
				m_collection = newCollection;
				SignalOrReset();
				return std::tuple(itemCountRemoved, originalCount);
			}

			virtual void SignalOrReset()
			{
				if (m_collection.size() == 0)
					m_hasMessages.Reset();
				else
					m_hasMessages.Signal();
			}

		protected:
			std::vector<T> m_collection;
			CRITICAL_SECTION m_criticalSection;
			Event m_hasMessages;
	};
}