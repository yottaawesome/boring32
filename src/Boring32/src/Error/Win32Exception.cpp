#include "pch.hpp"
#include "include/Error/Error.hpp"
#include "include/Strings/Strings.hpp"

namespace Boring32::Error
{
	Win32Exception::~Win32Exception() {}

	Win32Exception::Win32Exception(const char* msg, const DWORD errorCode)
		: std::runtime_error(msg),
		m_errorCode(errorCode)
	{
		m_errorString = Boring32::Error::CreateErrorStringFromCode(msg, errorCode);
	}

	Win32Exception::Win32Exception(const std::string& msg, const DWORD errorCode)
		: std::runtime_error(msg),
		m_errorCode(errorCode)
	{
		m_errorString = Boring32::Error::CreateErrorStringFromCode(msg, errorCode);
	}

	DWORD Win32Exception::GetErrorCode() const noexcept
	{
		return m_errorCode;
	}
	
	const char* Win32Exception::what() const noexcept
	{
		return m_errorString.c_str();
	}
}
