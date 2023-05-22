export module boring32.async:sharedmap;
import <map>;
import :slimreadwritelock;

export namespace Boring32::Async
{
	template<typename K, typename V>
	class SharedMap final
	{
		public:
			~SharedMap() = default;
			SharedMap() = default;
			SharedMap(const SharedMap&) = delete;
			SharedMap(SharedMap&&) noexcept = delete;
			SharedMap& operator=(const SharedMap&) = delete;
			SharedMap& operator=(SharedMap&&) noexcept = delete;

		public:
			void SetValue(const K& key, const V& value)
			{
				ExclusiveLockScope(m_lock.GetLock());
				m_map[key] = value;
			}

			V GetValueCopy(const K& key) requires std::is_default_constructible_v<V>
			{
				SharedLockScope(m_lock.GetLock());
				if(m_map.contains(key))
					return m_map[key];
				return {};
			}

			bool GetValueCopy(const K& key, V& out)
			{
				SharedLockScope(m_lock.GetLock());
				if (m_map.contains(key))
				{
					out = m_map[key];
					return true;
				}
				return false;
			}

			bool ExecuteOnValue(const K& key, const auto& function)
			{
				SharedLockScope(m_lock.GetLock());
				if (m_map.contains(key))
				{
					function(m_map[key]);
					return true;
				}
				return false;
			}

		private:
			std::map<K, V> m_map;
			SlimReadWriteLock m_lock;
	};
}