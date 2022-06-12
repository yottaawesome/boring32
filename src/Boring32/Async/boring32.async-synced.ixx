module;

#include <utility>

export module boring32.async:synced;

export namespace Boring32::Async
{
	template<typename T>
	class Synced
	{
		public:
			template<typename...Args>
			Synced(Args... args)
				: m_protected(args...)
			{

			}

		protected:
			T m_protected;
	};
}