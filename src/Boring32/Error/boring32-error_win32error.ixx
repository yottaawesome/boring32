export module boring32:error_win32error;
import std;
import boring32.win32;
import :error_boring32error;
import :error_functions;

export namespace Boring32::Error
{
	struct Win32Error final : Boring32Error
	{
		Win32Error(
			const std::string& msg,
			const std::source_location location = std::source_location::current(),
			const std::stacktrace& trace = std::stacktrace::current()
		) : Boring32Error(ExactMessage{ Error::FormatErrorMessage(
				"Win32",
				trace,
				location,
				msg
			) })
		{ }

		Win32Error(
			const unsigned long errorCode,
			const std::string& msg,
			const std::source_location location = std::source_location::current(),
			const std::stacktrace & trace = std::stacktrace::current()
		) : m_errorCode(errorCode), Boring32Error(Generate(msg, errorCode, location, trace))
		{ }

		template<typename...Args>
		Win32Error(
			const unsigned long errorCode,
			const std::string& msg,
			const std::source_location location = std::source_location::current(),
			const std::stacktrace& trace = std::stacktrace::current(),
			const Args&...args
		) : m_errorCode(errorCode), Boring32Error(Generate(msg, errorCode, location, trace, args...))
		{ }

		Win32Error(
			const unsigned long errorCode,
			const std::string& msg, 
			const std::wstring& moduleName,
			const std::source_location location = std::source_location::current(),
			const std::stacktrace& trace = std::stacktrace::current()
		) : m_errorCode(errorCode), Boring32Error(Generate(msg, errorCode, moduleName, location, trace))
		{ }

		unsigned long GetErrorCode() const noexcept
		{
			return m_errorCode;
		}

	private:
		ExactMessage Generate(
			const std::string& msg,
			const unsigned long errorCode,
			const std::wstring& moduleName,
			const std::source_location location,
			const std::stacktrace& trace
		)
		{
			std::string m_message = Error::TranslateErrorCode<std::string>(
				errorCode,
				moduleName
			);
			m_message = Error::FormatErrorMessage(
				"Win32",
				trace,
				location,
				msg,
				errorCode,
				m_message
			);

			return { m_message };
		}

		ExactMessage Generate(
			const std::string& msg,
			const unsigned long errorCode,
			const std::source_location location = std::source_location::current(),
			const std::stacktrace& trace = std::stacktrace::current()
		)
		{
			std::string m_message = Boring32::Error::TranslateErrorCode<std::string>(errorCode);
			return {
				Error::FormatErrorMessage(
					"Win32",
					trace,
					location,
					msg,
					errorCode,
					m_message
				) 
			};
		}

		template<typename...Args>
		ExactMessage Generate(
			const std::string& msg,
			const unsigned long errorCode,
			const std::source_location location = std::source_location::current(),
			const std::stacktrace& trace = std::stacktrace::current(),
			const Args&...args
		)
		{
			std::string format = std::vformat(
				msg,
				std::make_format_args(args...)
			);
			std::string m_message =
				Boring32::Error::TranslateErrorCode<std::string>(errorCode);
			return { 
				Error::FormatErrorMessage(
					"Win32",
					trace,
					location,
					msg,
					errorCode,
					m_message
				) 
			};
		}

		unsigned long m_errorCode = 0;
	};
}