// See #include <winnt.h> for FAST_FAIL error codes
module boring32.error:win32error;
import :functions;
import <format>;

namespace Boring32::Error
{
	Win32Error::Win32Error(
		const std::string& msg,
		const std::source_location location
	) 
	{
		m_message = Error::FormatErrorMessage(
			"Win32", 
			location, msg
		);
	}

	Win32Error::Win32Error(
		const std::string& msg,
		const unsigned long errorCode,
		const std::source_location location
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

	Win32Error::Win32Error(
		const std::string& msg,
		const unsigned long errorCode,
		const std::wstring& moduleName,
		const std::source_location location
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

	unsigned long Win32Error::GetErrorCode() const noexcept
	{
		return m_errorCode;
	}
}
