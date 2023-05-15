export module boring32.error:errorbase;
import <stdexcept>;
import <format>;
import <string>;
import <source_location>;
import <stacktrace>;

export namespace Boring32::Error
{
	template<typename T> requires std::is_base_of<std::exception, T>::value
	class ErrorBase : public T
	{
		public:
			virtual ~ErrorBase() = default;
			ErrorBase() = delete;
			ErrorBase(const ErrorBase&) = default;
			ErrorBase(ErrorBase&&) noexcept = default;
			virtual ErrorBase& operator=(const ErrorBase&) = default;
			virtual ErrorBase& operator=(ErrorBase&&) noexcept = default;

		public:
			//template <std::enable_if<std::is_default_constructible<T>::value, bool> = true>
			ErrorBase(
				const std::source_location& location = std::source_location::current(),
				const std::stacktrace& trace = std::stacktrace::current()
			) requires std::is_default_constructible_v<T>
				: m_location(location),
				m_trace(trace)
			{
				SetErrorMessage();
			}

			template<typename...Args>
			ErrorBase(
				const std::source_location& location, 
				const std::stacktrace& trace,
				const Args&... args
			)
				: T(std::forward<Args>(args)...), 
				m_location(location),
				m_trace(trace)
			{
				SetErrorMessage();
			}

		public:
			virtual const char* what() const noexcept override
			{ 
				return m_errorMsg.c_str();
			}

			virtual std::source_location& GetLocation() const noexcept
			{
				return m_location;
			}

			virtual std::stacktrace& GetStacktrace() const noexcept
			{
				return m_trace;
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
			std::stacktrace m_trace;
	};
}

namespace Boring32::Error
{
	template<typename T>
	concept CheckStringConstructor = requires()
	{
		T("dummy-value");
	};

	template<typename...E>
	[[deprecated("This is just here for possible repurposing.")]]
	class Error : public virtual E...
	{
		public:
			virtual ~Error() {}
	};

	class M [[deprecated("This is just here for possible repurposing.")]]
		: public Error<std::runtime_error, std::exception>
	{ };
}
