export module boring32:error_comerror;
import boring32.shared;
import :error_boring32error;
import :error_functions;

export namespace Boring32::Error
{
	struct COMError : public Boring32Error
	{
		virtual ~COMError() = default;
		COMError() = delete;
		COMError(const COMError& other) = default;
		virtual COMError& operator=(const COMError& other) = default;
		COMError(COMError&& other) noexcept = default;
		virtual COMError& operator=(COMError&& other) noexcept = default;

		COMError(
			const std::string& msg, 
			const Win32::HRESULT hr,
			const std::source_location location = std::source_location::current(),
			const std::stacktrace& trace = std::stacktrace::current()
		) : m_hresult(hr), Boring32Error(GenerateErrorMessage(location, msg, trace))
		{ }

		virtual Win32::HRESULT GetHResult() const noexcept
		{
			return m_hresult;
		}

		private:
		ExactMessage GenerateErrorMessage(
			const std::source_location& location,
			const std::string& message,
			const std::stacktrace& trace
		)
		{
			std::string m_errorString = Error::TranslateErrorCode<std::string>(
				m_hresult
			);
			m_errorString = Error::FormatErrorMessage(
				"COM", 
				trace,
				location,
				message, 
				m_hresult, 
				m_errorString
			);
			return { m_errorString };
		}

		protected:
		Win32::HRESULT m_hresult = 0;
	};
}