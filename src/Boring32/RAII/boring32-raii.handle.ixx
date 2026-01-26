export module boring32:raii_handle;
import std;
import :win32;
//https://docs.microsoft.com/en-us/windows/win32/sysinfo/kernel-objects

export namespace Boring32::RAII
{
	template<class T>	
	struct HandleTraits
	{
		using Type = T;
		using TypeConstReference = const T&;
		using TypePointer = T*;

		static bool IsValid(const T handle) { return false; };

		static void Close(T handle) {  };

		static constexpr bool IsCopyAssignable = false;

		static void CopyAssignValue(T left, T right) {  };
	};

	template<>
	struct HandleTraits<Win32::HANDLE>
	{
		using Type = Win32::HANDLE;
		using TypeConstReference = const Win32::HANDLE&;
		using TypePointer = Win32::HANDLE*;

		static bool IsValid(const HANDLE handle)
		{
			return handle != 0 && handle != Win32::InvalidHandleValue;
		};

		static void Close(Win32::HANDLE handle)
		{
			Win32::CloseHandle(handle);
		}

		static constexpr bool IsCopyAssignable = true;

		static void CopyAssignValue(Win32::HANDLE left, Win32::HANDLE right)
		{
			
		}
	};

	template<typename T>
	class Handle
	{
		public:
			virtual void Close()
			{
				if (HandleTraits<T>::IsValid(m_handle))
					HandleTraits<T>::Close(m_handle);
			}

			/*template <typename A = HandleTraits<T>::Type, std::enable_if_t<std::is_integral<A>::value, bool> = true>*/
			template <typename A = X> requires std::is_integral<A>::value
			void Blah()
			{

			}
			
		public:
			virtual operator bool()
			{
				return HandleTraits<T>::IsValid(m_handle);
			}

			template <typename A = T> requires HandleTraits<A>::IsCopyAssignable
			Handle<A>& operator=(const typename HandleTraits<A>::Type handle)
			{
				if (handle == m_handle)
					return *this;

				Close();
				HandleTraits<A>::CopyAssignValue(m_handle, handle);
				return *this;
			}

		protected:
			typename HandleTraits<T>::Type m_handle;
	};

	using BasicHandle = Handle<Win32::HANDLE>;
}