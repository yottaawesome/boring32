export module boring32:async_threadsafevector;
import std;
import boring32.win32;
import :async_criticalsection;
import :async_event;
import :concepts;

export namespace Boring32::Async
{
	template<typename TFn, typename TArg>
	concept FindFn = Concepts::Signature<TFn, bool, TArg>;

	// There's a compiler ICE when refactoring to use concepts and auto&&.
	// https://developercommunity.visualstudio.com/t/ICE-with-templates-and-explicit-object-p/10950903
	template<typename T, typename TLockType>
	struct ThreadSafeVector final
	{
		ThreadSafeVector() = default;
		ThreadSafeVector(const ThreadSafeVector&) = delete;
		ThreadSafeVector operator=(const ThreadSafeVector&) = delete;

		ThreadSafeVector(std::convertible_to<T> auto&&... args)
		{
			Add(std::forward<decltype(args)>(args)...);
		}

		void Add(std::convertible_to<T> auto&&...msg) noexcept
			requires std::copy_constructible<T>
		{
			std::scoped_lock<TLockType> cs(m_criticalSection);
			(m_collection.push_back(std::forward<decltype(msg)>(msg)), ...);
			m_hasMessages.Signal();
		}

		auto Size() const -> size_t
		{
			std::scoped_lock<TLockType> cs(m_criticalSection);
			return m_collection.size();
		}

		auto ToVector() const -> std::vector<T>
			requires std::copy_constructible<T>
		{
			std::scoped_lock<TLockType> cs(m_criticalSection);
			return std::vector<T>(m_collection);
		}

		void Clear()
		{
			std::scoped_lock<TLockType> cs(m_criticalSection);
			m_collection.clear();
			m_hasMessages.Reset();
		}

		void DoWithLock(std::invocable<std::vector<T>&> auto&& func)
		{
			std::scoped_lock<TLockType> cs(m_criticalSection);
			func(m_collection);
			SignalOrReset();
		}

		auto At(int index) -> T requires std::copy_constructible<T>
		{
			std::scoped_lock<TLockType> cs(m_criticalSection);
			return index < m_collection.size()
				? m_collection[index]
				: throw std::runtime_error(std::format("Index {} is out of bounds for size {}.", index, m_collection.size()));
		}

		void ForEach(FindFn<T> auto&& func)
		{
			std::scoped_lock<TLockType> cs(m_criticalSection);
			for (T& item : m_collection)
				if (not func(item))
					break;
		}

		void ForEachThenClear(std::invocable<T> auto&& func)
		{
			std::scoped_lock<TLockType> cs(m_criticalSection);
			for (T& item : m_collection)
				func(item);
			Clear();
		}

		// Delete items where func(item) returns true.
		auto DeleteWhere(FindFn<T> auto&& func) -> std::tuple<size_t, size_t>
		{
			std::scoped_lock<TLockType> cs(m_criticalSection);
			size_t originalCount = m_collection.size();
			std::vector<int> indexesToKeep;
			for (int i = 0; i < m_collection.size(); i++)
			{
				if (not func(m_collection[i]))
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

		auto DeleteOne(FindFn<T> auto&& findFunc) -> bool
		{
			std::scoped_lock<TLockType> cs(m_criticalSection);
			if (auto index = IndexOf(findFunc); index)
			{
				DeleteAt(*index);
				if (m_collection.size() == 0)
					m_hasMessages.Reset();
				return true;
			}
			return false;
		}

		auto ExtractOne(FindFn<T> auto&& findFunc) -> std::optional<T>
		{
			std::scoped_lock<TLockType> cs(m_criticalSection);
			if (auto index = IndexOf(findFunc); index)
			{
				T itemToSet = m_collection[*index];
				DeleteAt(*index);
				if (m_collection.size() == 0)
					m_hasMessages.Reset();
				return itemToSet;
			}
			return std::nullopt;
		}

		void DeleteAt(size_t index)
		{
			if (index >= m_collection.size())
				return;

			std::scoped_lock<TLockType> cs(m_criticalSection);
			m_collection.erase(m_collection.begin() + index);
			if (m_collection.size() == 0)
				m_hasMessages.Reset();
		}

		auto IndexOf(Concepts::Signature<bool, T> auto&& func) -> std::optional<size_t>
		{
			std::scoped_lock<TLockType> cs(m_criticalSection);
			auto iter = std::find_if(m_collection.begin(), m_collection.end(), func);
			if (iter == m_collection.end())
				return std::nullopt;
			return std::distance(m_collection.begin(), iter);
		}

		auto GetWaitableHandle() noexcept -> Win32::HANDLE
		{
			return m_hasMessages.GetHandle();
		}

	private:
		void SignalOrReset()
		{
			if (m_collection.size() == 0)
				m_hasMessages.Reset();
			else
				m_hasMessages.Signal();
		}

		std::vector<T> m_collection;
		mutable TLockType m_criticalSection{ 0 }; // For const functions.
		ManualResetEvent m_hasMessages{ false, false };
	};
}
