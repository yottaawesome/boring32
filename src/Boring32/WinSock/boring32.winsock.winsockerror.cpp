module;

#include <string>
#include <source_location>
#include <stdexcept>
#include <Windows.h>

module boring32.winsock.winsockerror;
import boring32.error.functions;

namespace Boring32::WinSock
{
	WinSockError::~WinSockError() { }

	WinSockError::WinSockError(
		const std::source_location& location, 
		const std::string& message, 
		const DWORD errorCode
	)
		: std::runtime_error(""),
		m_errorCode(errorCode)
	{
		m_errorString = Boring32::Error::TranslateErrorCode<std::string>(errorCode, L"Ws2_32.dll");
		m_errorString = Error::FormatErrorMessage(location, message, "winsock", errorCode, m_errorString);
	}

	DWORD WinSockError::GetErrorCode() const noexcept
	{
		return m_errorCode;
	}

	const char* WinSockError::what() const noexcept
	{
		return m_errorString.c_str();
	}
}