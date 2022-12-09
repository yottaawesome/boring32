module;

#include <source_location>

module boring32.error:comerror;
import :functions;
import <format>;

namespace Boring32::Error
{
	COMError::~COMError() {}

	COMError::COMError(const COMError& other)					= default;
	COMError::COMError(COMError&& other) noexcept				= default;
	COMError& COMError::operator=(const COMError& other)		= default;
	COMError& COMError::operator=(COMError&& other) noexcept	= default;

	COMError::COMError(
		const std::string& msg,
		const HRESULT hr,
		const std::source_location location
	)
		: Boring32Error(),
		m_hresult(hr)
	{
		GenerateErrorMessage(location, msg);
	}

	HRESULT COMError::GetHResult() const noexcept
	{
		return m_hresult;
	}

	const char* COMError::what() const noexcept
	{
		return m_errorString.c_str();
	}

	void COMError::GenerateErrorMessage(
		const std::source_location& location,
		const std::string& message
	)
	{
		m_errorString = Boring32::Error::TranslateErrorCode<std::string>(m_hresult);
		m_errorString = Error::FormatErrorMessage("COM", location, message, m_hresult, m_errorString);
	}
}
