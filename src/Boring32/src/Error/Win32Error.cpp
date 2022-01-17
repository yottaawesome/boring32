module;

#include <stdexcept>
#include <source_location>
#include <format>
// See #include <winnt.h> for FAST_FAIL error codes
#include "Windows.h"

module boring32.error.win32error;
import boring32.error.functions;

namespace Boring32::Error
{
	Win32Error::~Win32Error() {}

	Win32Error::Win32Error(const std::source_location& location, const char* msg)
		: std::runtime_error(msg),
		m_errorCode(0)
	{
		m_errorString = std::format(
			"Exception in function {}() in {}:{}:{}, {} (no win32 code provided)",
			location.function_name(),
			location.file_name(),
			location.line(),
			location.column(),
			msg
		);
	}

	Win32Error::Win32Error(const char* msg, const DWORD errorCode)
		: std::runtime_error(msg),
		m_errorCode(errorCode)
	{
		m_errorString = Boring32::Error::TranslateErrorCode<std::string>(errorCode);
		m_errorString = std::format("{} (win32 code: {}, {:#X}): {}", msg, errorCode, errorCode, m_errorString);
	}

	Win32Error::Win32Error(const std::string& msg, const DWORD errorCode)
		: std::runtime_error(msg),
		m_errorCode(errorCode)
	{
		m_errorString = Boring32::Error::TranslateErrorCode<std::string>(errorCode);
		m_errorString = std::format("{} (win32 code: {}, {:#X}): {}", msg, errorCode, errorCode, m_errorString);
	}
	
	Win32Error::Win32Error(const std::source_location& location, const std::string& msg, const DWORD errorCode)
		: std::runtime_error(msg),
		m_errorCode(errorCode)
	{
		m_errorString = Boring32::Error::TranslateErrorCode<std::string>(errorCode);
		m_errorString = std::format(
			"Exception in function {}() in {}:{}:{}, {}, win32 error code {} ({:#X}): {}",
			location.function_name(),
			location.file_name(),
			location.line(),
			location.column(),
			msg, 
			errorCode, 
			errorCode, 
			m_errorString
		);
	}

	DWORD Win32Error::GetErrorCode() const noexcept
	{
		return m_errorCode;
	}
	
	const char* Win32Error::what() const noexcept
	{
		return m_errorString.c_str();
	}
}
