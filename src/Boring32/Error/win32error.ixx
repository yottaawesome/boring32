export module boring32:error.win32error;
import std;
import :win32;
import :error.boring32error;
import :error.functions;

export namespace Boring32::Error
{
	class Win32Error final : public Boring32Error
	{
	public:
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
			Win32::DWORD errorCode,
			const std::string& msg,
			const std::source_location location = std::source_location::current(),
			const std::stacktrace & trace = std::stacktrace::current()
		) : m_errorCode(errorCode), Boring32Error(Generate(msg, errorCode, location, trace))
		{ }

		template<typename...Args>
		Win32Error(
			Win32::DWORD errorCode,
			const std::string& msg,
			const std::source_location location = std::source_location::current(),
			const std::stacktrace& trace = std::stacktrace::current(),
			const Args&...args
		) : m_errorCode(errorCode), Boring32Error(Generate(msg, errorCode, location, trace, args...))
		{ }

		Win32Error(
			Win32::DWORD errorCode,
			const std::string& msg, 
			const std::wstring& moduleName,
			const std::source_location location = std::source_location::current(),
			const std::stacktrace& trace = std::stacktrace::current()
		) : m_errorCode(errorCode), Boring32Error(Generate(msg, errorCode, moduleName, location, trace))
		{ }

		auto GetErrorCode() const noexcept -> Win32::DWORD
		{
			return m_errorCode;
		}

	private:
		auto Generate(
			const std::string& msg,
			Win32::DWORD errorCode,
			const std::wstring& moduleName,
			const std::source_location location,
			const std::stacktrace& trace
		) -> ExactMessage
		{
			auto message = Error::TranslateErrorCode<std::string>(errorCode, moduleName);
			message = Error::FormatErrorMessage(
				"Win32",
				trace,
				location,
				msg,
				errorCode,
				message
			);
			return { message };
		}

		auto Generate(
			const std::string& msg,
			Win32::DWORD errorCode,
			const std::source_location location = std::source_location::current(),
			const std::stacktrace& trace = std::stacktrace::current()
		) -> ExactMessage
		{
			auto message = Boring32::Error::TranslateErrorCode<std::string>(errorCode);
			return {
				Error::FormatErrorMessage(
					"Win32",
					trace,
					location,
					msg,
					errorCode,
					message
				) 
			};
		}

		template<typename...Args>
		auto Generate(
			const std::string& msg,
			Win32::DWORD errorCode,
			const std::source_location location = std::source_location::current(),
			const std::stacktrace& trace = std::stacktrace::current(),
			const Args&...args
		) -> ExactMessage
		{
			auto format = std::vformat(msg, std::make_format_args(args...));
			auto message = Boring32::Error::TranslateErrorCode<std::string>(errorCode);
			return { 
				Error::FormatErrorMessage(
					"Win32",
					trace,
					location,
					msg,
					errorCode,
					message
				) 
			};
		}

		Win32::DWORD m_errorCode = 0;
	};

	using Win32Expected = std::expected<void, Error::Win32Error>;
}
