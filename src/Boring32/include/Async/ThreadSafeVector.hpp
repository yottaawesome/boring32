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
				: m_hasMessages(true, false, true, false, L"")
			{
				InitializeCriticalSection(&m_criticalSection);
			}

			virtual void Add(T&& msg) noexcept
			{
				CriticalSectionLock cs(m_criticalSection);
				m_messages.push_back(msg);
				m_hasMessages.Signal();
			}

			virtual void Add(T& msg)
			{
				CriticalSectionLock cs(m_criticalSection);
				m_messages.push_back(msg);
				m_hasMessages.Signal();
			}

			virtual size_t Size()
			{
				CriticalSectionLock cs(m_criticalSection);
				return m_messages.size();
			}

			virtual void Clear()
			{
				CriticalSectionLock cs(m_criticalSection);
				m_messages.clear();
				m_hasMessages.Reset();
			}

			virtual void DoWithLock(std::function<void(std::vector<T>&)>& func)
			{
				CriticalSectionLock cs(m_criticalSection);
				func(m_messages);
			}

			virtual void DoWithLock(std::function<void(std::vector<T>&)>&& func)
			{
				CriticalSectionLock cs(m_criticalSection);
				func(m_messages);
			}

			virtual T CopyOfElementAt(const int index)
			{
				CriticalSectionLock cs(m_criticalSection);
				return m_messages.at(index);
			}

			virtual void ForEach(std::function<bool(T&)>& func)
			{
				CriticalSectionLock cs(m_criticalSection);
				for (T& item : m_messages)
					if (func(item) == false)
						break;;
			}

			virtual void ForEach(std::function<bool(T&)>&& func)
			{
				CriticalSectionLock cs(m_criticalSection);
				for (T& item : m_messages)
					if (func(item) == false)
						break;;
			}

			virtual void ForEachAndClear(std::function<void(T&)>& func)
			{
				CriticalSectionLock cs(m_criticalSection);
				for (T& item : m_messages)
					func(item);
				Clear();
			}

			virtual void ForEachAndClear(std::function<void(T&)>&& func)
			{
				CriticalSectionLock cs(m_criticalSection);
				for (T& item : m_messages)
					func(item);
				Clear();
			}

			virtual void ForEachAndClear(std::function<void(T*)>& func)
			{
				CriticalSectionLock cs(m_criticalSection);
				for (T& item : m_messages)
					func(&item);
				Clear();
			}

			virtual void ForEachAndClear(std::function<void(T*)>&& func)
			{
				CriticalSectionLock cs(m_criticalSection);
				for (T& item : m_messages)
					func(&item);
				Clear();
			}

			virtual bool EraseOne(const std::function<bool(const T&)>& findFunc)
			{
				CriticalSectionLock cs(m_criticalSection);

				int index = IndexOf(findFunc);
				if (index > -1)
				{
					RemoveAt(index);
					if (m_messages.size() == 0)
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
					if (m_messages.size() == 0)
						m_hasMessages.Reset();
				}
			}

			virtual bool FindAndErase(const std::function<bool(const T&)>& findFunc, T& itemToSet)
			{
				CriticalSectionLock cs(m_criticalSection);
				int index = IndexOf(findFunc);
				if (index > -1)
				{
					itemToSet = m_messages.at(index);
					RemoveAt(index);
					if (m_messages.size() == 0)
						m_hasMessages.Reset();
					return true;
				}
				return false;
			}

			virtual void RemoveAt(const size_t index)
			{
				CriticalSectionLock cs(m_criticalSection);
				if (index < m_messages.size())
				{
					m_messages.erase(m_messages.begin() + index);
					if (m_messages.size() == 0)
						m_hasMessages.Reset();
				}
			}

			virtual int IndexOf(const std::function<bool(const T&)>& func)
			{
				CriticalSectionLock cs(m_criticalSection);
				for (int i = 0; i < m_messages.size(); i++)
				{
					if (func(m_messages[i]))
						return i;
				}
				return -1;
			}

			virtual HANDLE GetWaitableHandle()
			{
				return m_hasMessages.GetHandle();
			}

		protected:
			std::vector<T> m_messages;
			CRITICAL_SECTION m_criticalSection;
			Event m_hasMessages;
	};
}