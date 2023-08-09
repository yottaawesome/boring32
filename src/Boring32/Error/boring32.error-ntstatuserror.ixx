export module boring32.error:ntstatuserror;
import std;
import :boring32error;
import :functions;

export namespace Boring32::Error
{
	// https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
	class NTStatusError : public Boring32Error
	{
		public:
			virtual ~NTStatusError() = default;
			NTStatusError(const NTStatusError& other) = default;
			virtual NTStatusError& operator=(const NTStatusError& other) = default;
			NTStatusError(NTStatusError&& other) noexcept = default;
			virtual NTStatusError& operator=(NTStatusError&& other) noexcept = default;

		public:
			// Related: https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
			NTStatusError(
				const std::string& msg,
				const std::source_location location = std::source_location::current(),
				const std::stacktrace& trace = std::stacktrace::current()
			) : m_errorCode(0)
			{
				GenerateErrorMessage(location, msg, trace);
			}

			NTStatusError(
				const std::string& msg,
				const long errorCode,
				const std::source_location location = std::source_location::current(),
				const std::stacktrace& trace = std::stacktrace::current()
			) : m_errorCode(errorCode)
			{
				GenerateErrorMessage(location, msg, trace);
			}

		public:
			[[nodiscard]] virtual long GetErrorCode() const noexcept
			{
				return m_errorCode;
			}

		protected:
			virtual void GenerateErrorMessage(
				const std::source_location& location,
				const std::string& message,
				const std::stacktrace& trace
			) override
			{
				if (m_errorCode)
				{
					m_message = Boring32::Error::GetNtStatusCode<std::string>(m_errorCode);
					m_message = Error::FormatErrorMessage(
						"NTSTATUS", trace,
						location, 
						message, 
						m_errorCode, 
						m_message
					);
				}
				else
				{
					m_message = Error::FormatErrorMessage(
						"NTSTATUS", 
						trace, 
						location, 
						message
					);
				}
			}

		protected:
			long m_errorCode = 0;
	};
}