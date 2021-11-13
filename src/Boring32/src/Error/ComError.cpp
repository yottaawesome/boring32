#include "pch.hpp"
#include "include/Error/Error.hpp"

namespace Boring32::Error
{
	ComError::~ComError() {}

	ComError::ComError(const char* msg, const HRESULT hr)
		: std::runtime_error(""),
		m_hresult(hr)
	{
		m_errorString = Boring32::Error::GetErrorFromHResult(msg, hr);
	}

	ComError::ComError(const std::string& msg, const HRESULT hr)
		: std::runtime_error(""),
		m_hresult(hr)
	{
		m_errorString = Boring32::Error::GetErrorFromHResult(msg, hr);
	}

	HRESULT ComError::GetHResult() const noexcept
	{
		return m_hresult;
	}

	const char* ComError::what() const noexcept
	{
		return m_errorString.c_str();
	}
}
