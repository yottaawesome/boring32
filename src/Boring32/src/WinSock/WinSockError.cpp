module;

#include "pch.hpp"
#include <string>

module boring32.winsock;
import boring32.error.functions;

namespace Boring32::WinSock
{
	WinSockError::~WinSockError() { }

	WinSockError::WinSockError(const DWORD errorCode)
		: std::runtime_error(""),
		m_errorCode(errorCode)
	{
		/*const HMODULE winsock = GetModuleHandleW(L"Ws2_32.dll");
		if (winsock == nullptr)
		{
			m_errorString += "WinSockError failed to translate error code " + errorCode;
			return;
		}*/

		m_errorString = Boring32::Error::TranslateErrorCode<std::string>(errorCode, L"Ws2_32.dll");
	}

	WinSockError::WinSockError(const std::string& message, const DWORD errorCode)
		: std::runtime_error(""),
		m_errorCode(errorCode)
	{
		m_errorString = Boring32::Error::TranslateErrorCode<std::string>(errorCode, L"Ws2_32.dll");
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