export module boring32.async:sharedmap;
import <map>;

export namespace Boring32::Async
{
	template<typename K, typename V>
	class SharedMap final
	{
		private:
			std::map<K, V> m_map;
	};
}