module;

#include "pch.hpp"
#include "include/Error/Error.hpp"

module boring32.winsock;

namespace Boring32::WinSock
{
	WinSockError::~WinSockError() { }

	WinSockError::WinSockError(const DWORD errorCode)
		: std::runtime_error(""),
		m_errorCode(errorCode)
	{
		const HMODULE winsock = GetModuleHandleW(L"Ws2_32.dll");
		if (winsock == nullptr)
		{
			m_errorString += "WinSockError failed to translate error code " + errorCode;
			return;
		}

		Boring32::Error::TranslateErrorCode(winsock, errorCode, m_errorString);
	}

	WinSockError::WinSockError(const std::string& message, const DWORD errorCode)
		: std::runtime_error(""),
		m_errorCode(errorCode)
	{
		const HMODULE winsock = GetModuleHandleW(L"Ws2_32.dll");
		if (winsock == nullptr)
		{
			m_errorString += "WinSockError failed to translate error code " + errorCode;
			return;
		}

		std::string temp;
		Boring32::Error::TranslateErrorCode(winsock, errorCode, temp);
		m_errorString += " " + temp;
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