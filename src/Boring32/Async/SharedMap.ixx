export module boring32:async.sharedmap;
import std;
import :async.slimreadwritelock;

export namespace Boring32::Async
{
	template<typename K, typename V>
	requires std::copyable<V>
	class SharedMap final
	{
	public:
		SharedMap() = default;
		SharedMap(const SharedMap&) = delete;
		SharedMap(SharedMap&&) noexcept = delete;
		auto operator=(const SharedMap&) -> SharedMap& = delete;
		auto operator=(SharedMap&&) noexcept -> SharedMap& = delete;

		auto SetValue(const K& key, std::convertible_to<V> auto&& value)
		{
			auto scope = ExclusiveLockScope{ m_lock.GetLock() };
			m_map[key] = value;
		}

		[[nodiscard]]
		auto GetValueCopy(const K& key) -> V
			requires std::is_default_constructible_v<V>
		{
			auto scope = SharedLockScope{ m_lock.GetLock() };
			if (m_map.contains(key))
				return m_map[key];
			return {};
		}

		[[nodiscard]]
		auto GetValueCopy(const K& key, V& out) -> bool
		{
			auto scope = SharedLockScope{ m_lock.GetLock() };
			if (m_map.contains(key))
			{
				out = m_map[key];
				return true;
			}
			return false;
		}

		auto ExecuteOnValue(const K& key, std::invocable<const V&> auto&& function) -> bool
		{
			auto scope = SharedLockScope{ m_lock.GetLock() };
			if (m_map.contains(key))
			{
				function(m_map[key]);
				return true;
			}
			return false;
		}

		[[nodiscard]]
		auto operator[](const K& key) -> V
		{
			auto scope = SharedLockScope{ m_lock.GetLock() };
			if (m_map.contains(key))
				return m_map[key];
			return {};
		}

	private:
		std::map<K, V> m_map;
		SlimReadWriteLock m_lock;
	};
}