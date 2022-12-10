module boring32.error:ntstatuserror;
import :functions;
import <format>;

namespace Boring32::Error
{
	// Related: https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
	NtStatusError::~NtStatusError() {}

	NtStatusError::NtStatusError(const NtStatusError& other)				= default;
	NtStatusError::NtStatusError(NtStatusError&& other) noexcept			= default;
	NtStatusError& NtStatusError::operator=(const NtStatusError& other)		= default;
	NtStatusError& NtStatusError::operator=(NtStatusError&& other) noexcept	= default;

	NtStatusError::NtStatusError(
		const std::string& msg,
		const std::source_location location
	) : m_errorCode(0), Boring32Error(msg, location)
	{
		GenerateErrorMessage(location, msg);
	}

	NtStatusError::NtStatusError(
		const std::string& msg,
		const long errorCode,
		const std::source_location location
	) : m_errorCode(errorCode), Boring32Error(msg, location)
	{
		GenerateErrorMessage(location, msg);
	}

	long NtStatusError::GetErrorCode() const noexcept
	{
		return m_errorCode;
	}

	void NtStatusError::GenerateErrorMessage(
		const std::source_location& location,
		const std::string& message
	)
	{
		if (m_errorCode)
		{
			m_message = Boring32::Error::GetNtStatusCode<std::string>(m_errorCode);
			m_message = Error::FormatErrorMessage("NTSTATUS", location, message, m_errorCode, m_message);
		}
		else
		{
			m_message = Error::FormatErrorMessage("NTSTATUS", location, message);
		}
	}
}
