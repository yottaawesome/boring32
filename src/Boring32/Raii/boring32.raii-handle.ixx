module;

#include <type_traits>

export module boring32.raii:handle;

export namespace Boring32::Raii
{
	template<class T>	
	struct HandleTraits
	{
		using Type = T;
		using ConstTypeReference = const T&;
		static bool IsValid(const T& handle)
		{
			return handle;
		};

		static void Close(const T& handle)
		{

		}

		constexpr bool IsCopyAssignable() { return true; };
	};

	template<typename T>
	class Handle
	{
		public:
			virtual operator bool()
			{
				return HandleTraits<T>::IsValid(m_handle);
			}

			virtual void Close()
			{
				HandleTraits<T>::Close(m_handle);
			}

			/*template <typename A = HandleTraits<T>::Type,
				std::enable_if_t<std::is_integral<A>::value, bool> = true>*/
			template <typename A = X> requires std::is_integral<A>::value
			void Blah()
			{

			}
			
			template <typename A = T>
				requires HandleTraits<A>::IsCopyAssignable
			Handle<T>& operator=(const typename HandleTraits<T>::Type& handle)
			{
				//m_handle = handle;
				return *this;
			}

		protected:
			HandleTraits<T>::Type m_handle;
	};
}