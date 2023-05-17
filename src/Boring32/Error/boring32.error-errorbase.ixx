module;

#include <source_location>;

export module boring32.error:errorbase;
import <stdexcept>;
import <format>;
import <string>;
import <source_location>;
import <stacktrace>;
import :functions;
import :boring32error;

export namespace Boring32::Error
{
	template<typename T>
	concept CheckStringConstructor = requires()
	{
		T("dummy-value");
	};

	template<typename T> requires std::is_base_of_v<std::exception, T>
	class ErrorBase : public T
	{
		public:
			virtual ~ErrorBase() = default;
			ErrorBase() requires std::is_default_constructible_v<T> = default;
			ErrorBase(const ErrorBase&) = default;
			ErrorBase(ErrorBase&&) noexcept = default;
			virtual ErrorBase& operator=(const ErrorBase&) = default;
			virtual ErrorBase& operator=(ErrorBase&&) noexcept = default;

		public:
			//template <std::enable_if<std::is_default_constructible<T>::value, bool> = true>
			ErrorBase(
				MessageLocationTrace msg
			) requires std::is_default_constructible_v<T>
				: m_location(std::move(msg.Location)),
				m_trace(std::move(msg.Trace))
			{
				SetErrorMessage(msg);
			}

			template<typename...Args>
			ErrorBase(
				const std::string_view msg,
				const std::source_location& location,
				const std::stacktrace& trace,
				const Args&... args
			) requires !CheckStringConstructor<T>
				: T(args...),
				m_location(location),
				m_trace(trace)
			{
				SetErrorMessage(msg);
			}

			template<typename...Args> 
			ErrorBase(
				const std::string_view msg,
				const std::source_location& location = std::source_location::current(),
				const std::stacktrace& trace = std::stacktrace::current()
			) requires CheckStringConstructor<T>
				: T(msg.data()),
				m_location(location),
				m_trace(trace)
			{
				SetErrorMessage(msg);
			}

		public:
			virtual const char* what() const noexcept override
			{ 
				return m_errorMsg.c_str();
			}

			virtual const std::source_location& GetLocation() const noexcept
			{
				return m_location;
			}

			virtual const std::stacktrace& GetStacktrace() const noexcept
			{
				return m_trace;
			}

		protected:
			virtual void SetErrorMessage(const std::string_view msg)
			{
				m_errorMsg = FormatErrorMessage(
					m_trace,
					m_location,
					std::string(msg)
				);
			}

		protected:
			std::source_location m_location;
			std::stacktrace m_trace;
			std::string m_errorMsg;
	};

	using RuntimeError = ErrorBase<std::runtime_error>;
}

namespace Boring32::Error
{
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
