export module boring32:error.errorbase;
import std;
import :error.functions;
import :error.boring32error;

export namespace Boring32::Error
{
	template<typename T>
	concept CheckStringConstructor = requires()
	{
		T("dummy-value");
	};

	template<typename T> requires std::is_base_of_v<std::exception, T>
	struct ErrorBase : public T
	{
		virtual ~ErrorBase() = default;
		ErrorBase() requires std::is_default_constructible_v<T> = default;
		ErrorBase(const ErrorBase&) = default;
		ErrorBase(ErrorBase&&) noexcept = default;
		virtual ErrorBase& operator=(const ErrorBase&) = default;
		virtual ErrorBase& operator=(ErrorBase&&) noexcept = default;

		//template <std::enable_if<std::is_default_constructible<T>::value, bool> = true>
		ErrorBase(
			MessageLocationTrace msg
		) requires std::is_default_constructible_v<T>
			: m_location(std::move(msg.Location)),
			m_trace(std::move(msg.Trace))
		{
			SetErrorMessage(msg.Message);
		}

		template<typename...Args>
		ErrorBase(
			MessageLocationTrace msg,
			const Args&... args
		) requires !CheckStringConstructor<T>
			: T(args...),
			m_location(std::move(msg.Location)),
			m_trace(std::move(msg.Trace))
		{
			SetErrorMessage(msg.Message);
		}

		template<typename...Args> 
		ErrorBase(
			MessageLocationTrace msg
		) requires CheckStringConstructor<T>
			: T(msg.Message.data()),
			m_location(std::move(msg.Location)),
			m_trace(std::move(msg.Trace))
		{
			SetErrorMessage(msg.Message);
		}

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

		std::source_location m_location;
		std::stacktrace m_trace;
		std::string m_errorMsg;
	};

	using RuntimeError = ErrorBase<std::runtime_error>;
}

namespace Boring32::Error
{
	/*template<typename...E>
	struct Error [[deprecated("This is just here for possible repurposing.")]] : virtual E...
	{
		virtual ~Error() = default;
	};*/

	/*struct M [[deprecated("This is just here for possible repurposing.")]]
		: Error<std::runtime_error, std::exception>
	{ };*/
}
