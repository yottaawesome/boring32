export module boring32.async:threadsafevector;
import boring32.shared;
import :criticalsectionlock;
import :criticalsection;
import :event;

export namespace Boring32::Async
{
	// Todo: convert to slim-read-write lock
	template<typename T>
	class ThreadSafeVector final
	{
		public:
			ThreadSafeVector() = default;
			
			ThreadSafeVector(const ThreadSafeVector&) = delete;
			ThreadSafeVector operator=(const ThreadSafeVector&) = delete;

			void Add(T&& msg) noexcept
				requires std::is_move_constructible_v<T>
			{
				CriticalSectionLock cs(m_criticalSection.GetRef());
				m_collection.push_back(msg);
				m_hasMessages.Signal();
			}

			void Add(const T& msg)
				requires std::is_copy_constructible_v<T>
			{
				CriticalSectionLock cs(m_criticalSection.GetRef());
				m_collection.push_back(msg);
				m_hasMessages.Signal();
			}

			size_t Size() const
			{
				CriticalSectionLock cs(m_criticalSection.GetRef());
				return m_collection.size();
			}

			std::vector<T> ToVector() const
				requires std::is_copy_constructible_v<T>
			{
				CriticalSectionLock cs(m_criticalSection.GetRef());
				return std::vector<T>(m_collection);
			}

			void Clear()
			{
				CriticalSectionLock cs(m_criticalSection.GetRef());
				m_collection.clear();
				m_hasMessages.Reset();
			}

			void DoWithLock(std::function<void(std::vector<T>&)>& func)
			{
				CriticalSectionLock cs(m_criticalSection.GetRef());
				func(m_collection);
			}

			void DoWithLock(std::function<void(std::vector<T>&)>&& func)
			{
				CriticalSectionLock cs(m_criticalSection.GetRef());
				func(m_collection);
			}

			T CopyOfElementAt(const int index)
				requires std::is_copy_constructible_v<T>
			{
				CriticalSectionLock cs(m_criticalSection.GetRef());
				return m_collection.at(index);
			}

			void ForEach(const std::function<bool(T&)>& func)
			{
				CriticalSectionLock cs(m_criticalSection.GetRef());
				for (T& item : m_collection)
					if (func(item) == false)
						break;;
			}

			void ForEach(std::function<bool(T&)>&& func)
			{
				CriticalSectionLock cs(m_criticalSection.GetRef());
				for (T& item : m_collection)
					if (func(item) == false)
						break;
			}

			void ForEachAndClear(std::function<void(T&)>& func)
			{
				CriticalSectionLock cs(m_criticalSection.GetRef());
				for (T& item : m_collection)
					func(item);
				Clear();
			}

			void ForEachAndClear(std::function<void(T&)>&& func)
			{
				CriticalSectionLock cs(m_criticalSection.GetRef());
				for (T& item : m_collection)
					func(item);
				Clear();
			}

			void ForEachAndClear(std::function<void(T*)>& func)
			{
				CriticalSectionLock cs(m_criticalSection.GetRef());
				for (T& item : m_collection)
					func(&item);
				Clear();
			}

			void ForEachAndClear(std::function<void(T*)>&& func)
			{
				CriticalSectionLock cs(m_criticalSection.GetRef());
				for (T& item : m_collection)
					func(&item);
				Clear();
			}

			std::tuple<size_t, size_t> ForEachAndSelectiveClear(std::function<bool(T&)>& func)
			{
				return InternalForEachAndSelectiveClear(func);
			}

			std::tuple<size_t, size_t> ForEachAndSelectiveClear(std::function<bool(T&)>&& func)
			{
				return InternalForEachAndSelectiveClear(func);
			}

			bool EraseOne(const std::function<bool(const T&)>& findFunc)
			{
				CriticalSectionLock cs(m_criticalSection.GetRef());

				size_t index = IndexOf(findFunc);
				if (index > -1)
				{
					RemoveAt(index);
					if (m_collection.size() == 0)
						m_hasMessages.Reset();
				}
				return index > -1;
			}

			void EraseMultiple(const std::function<bool(const T&)>& findFunc)
			{
				CriticalSectionLock cs(m_criticalSection.GetRef());
				size_t index = 0;
				while ((index = IndexOf(findFunc)) != -1)
				{
					RemoveAt(index);
					if (m_collection.size() == 0)
						m_hasMessages.Reset();
				}
			}

			bool FindAndErase(const std::function<bool(const T&)>& findFunc, T& itemToSet)
			{
				CriticalSectionLock cs(m_criticalSection.GetRef());
				size_t index = IndexOf(findFunc);
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

			void RemoveAt(const size_t index)
			{
				if (index >= m_collection.size())
					return;

				CriticalSectionLock cs(m_criticalSection.GetRef());
				m_collection.erase(m_collection.begin() + index);
				if (m_collection.size() == 0)
					m_hasMessages.Reset();
			}

			size_t IndexOf(const std::function<bool(const T&)>& func)
			{
				CriticalSectionLock cs(m_criticalSection.GetRef());
				auto iter = std::find_if(m_collection.begin(), m_collection.end(), func);
				if (iter == m_collection.end())
					return -1;
				return std::distance(m_collection.begin(), iter);
			}

			Win32::HANDLE GetWaitableHandle() noexcept
			{
				return m_hasMessages.GetHandle();
			}

		private:
			std::tuple<size_t, size_t> InternalForEachAndSelectiveClear(std::function<bool(T&)>& func)
			{
				CriticalSectionLock cs(m_criticalSection.GetRef());
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
				m_collection = std::move(newCollection);
				SignalOrReset();
				return std::tuple(itemCountRemoved, originalCount);
			}

			void SignalOrReset()
			{
				if (m_collection.size() == 0)
					m_hasMessages.Reset();
				else
					m_hasMessages.Signal();
			}

		protected:
			std::vector<T> m_collection;
			mutable CriticalSection m_criticalSection{ 0 };
			Event m_hasMessages{ false, true, false };
	};
}