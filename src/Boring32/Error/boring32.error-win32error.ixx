export module boring32.error:win32error;
import <string>;
import <format>;
import <stdexcept>;
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
				const std::source_location location = std::source_location::current()
			)
			{
				m_message = Error::FormatErrorMessage(
					"Win32",
					location, msg
				);
			}

			Win32Error(
				const std::string& msg,
				const unsigned long errorCode,
				const std::source_location location = std::source_location::current()
			)
			{
				m_message = Boring32::Error::TranslateErrorCode<std::string>(errorCode);
				m_message = Error::FormatErrorMessage(
					"Win32",
					location,
					msg,
					errorCode,
					m_message
				);
			}

			Win32Error(
				const std::string& msg, 
				const unsigned long errorCode,
				const std::wstring& moduleName,
				const std::source_location location = std::source_location::current()
			)
			{
				m_message = Boring32::Error::TranslateErrorCode<std::string>(
					errorCode,
					moduleName
				);
				m_message = Error::FormatErrorMessage(
					"Win32",
					location,
					msg,
					errorCode,
					m_message
				);
			}		

		public:
			virtual unsigned long GetErrorCode() const noexcept
			{
				return m_errorCode;
			}

		protected:
			unsigned long m_errorCode = 0;
	};
}