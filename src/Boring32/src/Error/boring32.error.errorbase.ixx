module;

#include <stdexcept>
#include <format>
#include <string>
#include <source_location>

export module boring32.error.errorbase;

export namespace Boring32::Error
{
	template<typename T> requires std::is_base_of<std::exception, T>::value
	class ErrorBase : public T 
	{
		public:
			virtual ~ErrorBase() {}

			template <std::enable_if<std::is_default_constructible<T>::value, bool> = true>
			ErrorBase(const std::source_location& location) 
				: T(),
				m_location(location)
			{
				SetErrorMessage();
			}

			template<typename...Args>
			ErrorBase(const std::source_location& location, Args&... args)
				: T(std::forward<Args>(args)...), 
				m_location(location)
			{
				SetErrorMessage();
			}

		public:
			virtual const char* what() const noexcept 
			{ 
				return m_errorMsg.c_str();
			}

		protected:
			virtual void SetErrorMessage()
			{
				m_errorMsg = std::format(
					"Exception in function {}() at {}:{}:{} --> {}",
					m_location.function_name(),
					m_location.file_name(),
					m_location.line(),
					m_location.column(),
					T::what()
				);
			}

		protected:
			std::source_location m_location;
			std::string m_errorMsg;
	};


	template<typename...E>
	class Error : public virtual E...
	{
		public:
			virtual ~Error() {}
	};

	class M : public Error<std::runtime_error, std::exception>
	{

	};
}