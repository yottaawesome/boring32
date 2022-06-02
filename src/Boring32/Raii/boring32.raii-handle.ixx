module;

#include <type_traits>

export module boring32.raii:handle;

export namespace Boring32::Raii
{
	template<typename T>	
	struct HandleTraits
	{
		using Type = T;
		static bool IsValid(const T& handle)
		{
			return handle;
		};
	};

	template<typename T>
	class Handle
	{
		public:
			virtual operator bool()
			{
				return HandleTraits<T>::IsValid(m_handle);
			}

			template <typename A = HandleTraits<T>::Type,
				std::enable_if_t<std::is_integral<A>::value, bool> = true>
			void Blah()
			{

			}
			/*virtual Handle& operator=(const HandleTraits<T>::X handle)
			{
				m_handle = handle;
			}*/

		protected:
			HandleTraits<T>::Type m_handle;
	};
}