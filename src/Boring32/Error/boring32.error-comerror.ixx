export module boring32.error:comerror;
import <stdexcept>;
import <string>;
import <source_location>;
import <format>;
import <win32.hpp>;
import :boring32error;
import :functions;

export namespace Boring32::Error
{
	class COMError : public Boring32Error
	{
		public:
			virtual ~COMError() = default;
			COMError() = delete;
			COMError(const COMError& other) = default;
			virtual COMError& operator=(const COMError& other) = default;
			COMError(COMError&& other) noexcept = default;
			virtual COMError& operator=(COMError&& other) noexcept = default;

		public:
			COMError(
				const std::string& msg, 
				const HRESULT hr,
				const std::source_location location = std::source_location::current()
			) : m_hresult(hr)
			{
				GenerateErrorMessage(location, msg);
			}

		public:
			virtual HRESULT GetHResult() const noexcept
			{
				return m_hresult;
			}

			virtual const char* what() const noexcept override
			{
				return m_errorString.c_str();
			}

		protected:
			virtual void GenerateErrorMessage(
				const std::source_location& location,
				const std::string& message
			) override
			{
				m_errorString = Boring32::Error::TranslateErrorCode<std::string>(m_hresult);
				m_errorString = Error::FormatErrorMessage("COM", location, message, m_hresult, m_errorString);
			}

		protected:
			HRESULT m_hresult = 0;
			std::string m_errorString;
	};
}