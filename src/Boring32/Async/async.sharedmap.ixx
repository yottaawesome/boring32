export module boring32:async.sharedmap;
import std;
import :async.slimreadwritelock;

export namespace Boring32::Async
{
	template<typename K, typename V>
	struct SharedMap final
	{
		SharedMap() = default;
		SharedMap(const SharedMap&) = delete;
		SharedMap(SharedMap&&) noexcept = delete;
		auto operator=(const SharedMap&) -> SharedMap& = delete;
		auto operator=(SharedMap&&) noexcept -> SharedMap& = delete;

		auto SetValue(const K& key, const V& value) -> void
		{
			ExclusiveLockScope(m_lock.GetLock());
			m_map[key] = value;
		}

		auto GetValueCopy(const K& key) -> V requires std::is_default_constructible_v<V>
		{
			SharedLockScope(m_lock.GetLock());
			if(m_map.contains(key))
				return m_map[key];
			return {};
		}

		auto GetValueCopy(const K& key, V& out) -> bool
		{
			SharedLockScope(m_lock.GetLock());
			if (m_map.contains(key))
			{
				out = m_map[key];
				return true;
			}
			return false;
		}

		auto ExecuteOnValue(
			const K& key,
			// auto creates bad codegen that segfaults
			const std::function<void(const V&)>& function
		) -> bool
		{
			SharedLockScope(m_lock.GetLock());
			if (m_map.contains(key))
			{
				function(m_map[key]);
				return true;
			}
			return false;
		}

		auto operator[](const K& key) -> V
		{
			SharedLockScope(m_lock.GetLock());
			if (m_map.contains(key))
				return m_map[key];
			return {};
		}

		private:
		std::map<K, V> m_map;
		SlimReadWriteLock m_lock;
	};
}