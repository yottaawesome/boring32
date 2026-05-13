export module boring32:error.ntstatuserror;
import std;
import :win32;
import :error.boring32error;
import :error.functions;

export namespace Boring32::Error
{
	// https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
	class NTStatusError final : public Boring32Error
	{
	public:
		// Related: https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
		NTStatusError(
			const std::string& msg,
			const std::source_location location = std::source_location::current(),
			const std::stacktrace& trace = std::stacktrace::current()
		) : m_errorCode(0), Boring32Error(GenerateErrorMessage(location, msg, trace))
		{ }

		NTStatusError(
			const long errorCode,
			const std::string& msg,
			const std::source_location location = std::source_location::current(),
			const std::stacktrace& trace = std::stacktrace::current()
		) : m_errorCode(errorCode), Boring32Error(GenerateErrorMessage(location, msg, trace))
		{ }

		[[nodiscard]]
		auto GetErrorCode() const noexcept -> long
		{
			return m_errorCode;
		}

	private:
		auto GenerateErrorMessage(
			const std::source_location& location,
			const std::string& message,
			const std::stacktrace& trace
		) -> ExactMessage
		{
			if (m_errorCode)
			{
				auto message = Boring32::Error::GetNtStatusCode<std::string>(m_errorCode);
				message = Error::FormatErrorMessage(
					"NTSTATUS", trace,
					location, 
					message, 
					m_errorCode, 
					message
				);
			}

			return { Error::FormatErrorMessage("NTSTATUS", trace, location, message) };
		}

		long m_errorCode = 0;
	};
}