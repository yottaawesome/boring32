export module boring32.error:comerror;
import :boring32error;
import <stdexcept>;
import <string>;
import <source_location>;
import <win32.hpp>;

export namespace Boring32::Error
{
	class COMError : public Boring32Error
	{
		public:
			virtual ~COMError() = default;
			COMError(const COMError& other) = default;
			COMError(COMError&& other) noexcept = default;
			COMError(
				const std::string& msg, 
				const HRESULT hr,
				const std::source_location location = std::source_location::current()
			);

		public:
			virtual COMError& operator=(const COMError& other) = default;
			virtual COMError& operator=(COMError&& other) noexcept = default;

		public:
			virtual HRESULT GetHResult() const noexcept;
			virtual const char* what() const noexcept override;

		protected:
			virtual void GenerateErrorMessage(
				const std::source_location& location,
				const std::string& message
			) override;

		protected:
			HRESULT m_hresult = 0;
			std::string m_errorString;
	};
}