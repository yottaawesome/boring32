export module boring32.error:win32error;
import <string>;
import <format>;
import <stacktrace>;
import <source_location>;
import :boring32error;
import :functions;

export namespace Boring32::Error
{
	class Win32Error : public Boring32Error
	{
		public:
			virtual ~Win32Error() = default;
			Win32Error(const Win32Error& other) = default;
			virtual Win32Error& operator=(const Win32Error& other) = default;
			Win32Error(Win32Error&& other) noexcept = default;
			virtual Win32Error& operator=(Win32Error&& other) noexcept = default; 
			
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
			{
			}

			Win32Error(
				const std::string& msg,
				const unsigned long errorCode,
				const std::source_location location = std::source_location::current(),
				const std::stacktrace & trace = std::stacktrace::current()
			) : m_errorCode(errorCode), Boring32Error(Generate(msg, errorCode, location, trace))
			{ }

			template<typename...Args>
			Win32Error(
				const std::string& msg,
				const unsigned long errorCode,
				const std::source_location location = std::source_location::current(),
				const std::stacktrace& trace = std::stacktrace::current(),
				const Args&...args
			) : m_errorCode(errorCode), Boring32Error(Generate(msg, errorCode, location, trace, args...))
			{ }

			Win32Error(
				const std::string& msg, 
				const unsigned long errorCode,
				const std::wstring& moduleName,
				const std::source_location location = std::source_location::current(),
				const std::stacktrace& trace = std::stacktrace::current()
			) : m_errorCode(errorCode), Boring32Error(Generate(msg, errorCode, moduleName, location, trace))
			{ }

		public:
			virtual unsigned long GetErrorCode() const noexcept final
			{
				return m_errorCode;
			}

		protected:
			unsigned long m_errorCode = 0;

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
	};
}