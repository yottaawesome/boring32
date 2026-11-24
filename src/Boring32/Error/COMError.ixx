export module Boring32:Error.COMError;
import std;
import :Win32;
import :Error.Boring32Error;
import :Error.Functions;

export namespace Boring32::Error
{
	class COMError final : public Boring32Error
	{
	public:
		COMError() = delete;

		COMError(
			const Win32::HRESULT hr,
			const std::string& msg, 
			const std::source_location location = std::source_location::current(),
			const std::stacktrace& trace = std::stacktrace::current()
		) : m_hresult(hr), Boring32Error(GenerateErrorMessage(hr, location, msg, trace))
		{ }

		auto GetHResult() const noexcept -> Win32::HRESULT
		{
			return m_hresult;
		}

	private:
		auto GenerateErrorMessage(
			Win32::HRESULT hresult,
			const std::source_location& location,
			const std::string& message,
			const std::stacktrace& trace
		) -> ExactMessage
		{
			auto errorString = 
				Error::TranslateErrorCode<std::string>(hresult);
			errorString = Error::FormatErrorMessage(
				"COM", 
				trace,
				location,
				message, 
				hresult,
				errorString
			);
			return { errorString };
		}

		Win32::HRESULT m_hresult = 0;
	};
}