#include "pch.hpp"
#include "include/Win32Utils.hpp"

namespace Win32Utils::Error
{
	Win32Exception::~Win32Exception() {}

	Win32Exception::Win32Exception(const char* msg)
		: std::runtime_error(msg),
		m_errorCode(GetLastError())
	{ }

	Win32Exception::Win32Exception(const std::string& msg)
		: std::runtime_error(msg),
		m_errorCode(GetLastError())
	{ }

	Win32Exception::Win32Exception(const char* msg, DWORD errorCode)
		: std::runtime_error(msg),
		m_errorCode(errorCode)
	{ }

	Win32Exception::Win32Exception(const std::string& msg, DWORD errorCode)
		: std::runtime_error(msg),
		m_errorCode(errorCode)
	{ }

	DWORD Win32Exception::GetErrorCode() const
	{
		return m_errorCode;
	}

	std::wstring Win32Exception::GetErrorCodeWString() const
	{
		return Win32Utils::Error::GetErrorCodeWString(m_errorCode);
	}

	std::wstring Win32Exception::GetFullErrorWString() const
	{
		std::string whatString(this->what());
		std::wstring errorCodeWString(Win32Utils::Error::GetErrorCodeWString(m_errorCode));
		std::wstring finalErrorString(Strings::ConvertStringToWString(whatString));
		finalErrorString += L" :: ";
		finalErrorString += L"(";
		finalErrorString += std::to_wstring(m_errorCode);
		finalErrorString += L") ";
		finalErrorString += errorCodeWString;
		return finalErrorString;
	}
}
