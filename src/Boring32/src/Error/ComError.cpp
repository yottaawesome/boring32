#include "pch.hpp"
#include "include/Error/Error.hpp"
#include "include/Strings/Strings.hpp"

namespace Boring32::Error
{
	ComError::~ComError() {}

	ComError::ComError(const char* msg, const HRESULT errorCode)
		: std::runtime_error(msg),
		m_errorCode(errorCode)
	{
		m_errorString = Boring32::Error::GetErrorFromHResult(msg, errorCode);
	}

	ComError::ComError(const std::string& msg, const HRESULT errorCode)
		: std::runtime_error(msg),
		m_errorCode(errorCode)
	{
		m_errorString = Boring32::Error::GetErrorFromHResult(msg, errorCode);
	}

	HRESULT ComError::GetErrorCode() const noexcept
	{
		return m_errorCode;
	}

	const char* ComError::what() const noexcept
	{
		return m_errorString.c_str();
	}
}
