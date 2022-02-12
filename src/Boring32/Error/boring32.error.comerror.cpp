module;

#include <format>
#include <string>
#include <source_location>
#include <Windows.h>

module boring32.error.comerror;
import boring32.error.functions;

namespace Boring32::Error
{
	ComError::~ComError() {}

	ComError::ComError(const std::source_location& location, const std::string& msg, const HRESULT hr)
		: std::runtime_error(""),
		m_hresult(hr)
	{
		m_errorString = Boring32::Error::TranslateErrorCode<std::string>(hr);
		m_errorString = std::format("{} (COM code: {}, {:#X}): {}", msg, hr, hr, m_errorString);
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
