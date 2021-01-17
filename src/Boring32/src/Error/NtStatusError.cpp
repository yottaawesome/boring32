#include "pch.hpp"
#include "include/Error/Error.hpp"
#include "include/Strings/Strings.hpp"

namespace Boring32::Error
{
	// Related: https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
	NtStatusError::~NtStatusError() {}

	NtStatusError::NtStatusError(const char* msg, const LONG errorCode)
		: std::runtime_error(msg),
		m_errorCode(errorCode)
	{
		m_errorString = Boring32::Error::GetNtStatusError(msg, errorCode);
	}

	NtStatusError::NtStatusError(const std::string& msg, const LONG errorCode)
		: std::runtime_error(msg),
		m_errorCode(errorCode)
	{
		m_errorString = Boring32::Error::GetNtStatusError(msg, errorCode);
	}

	LONG NtStatusError::GetErrorCode() const noexcept
	{
		return m_errorCode;
	}

	const char* NtStatusError::what() const noexcept
	{
		return m_errorString.c_str();
	}
}
