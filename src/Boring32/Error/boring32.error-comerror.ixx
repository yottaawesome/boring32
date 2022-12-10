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
			virtual ~COMError();
			COMError(const COMError& other);
			COMError(COMError&& other) noexcept;
			COMError(
				const std::string& msg, 
				const HRESULT hr,
				const std::source_location location = std::source_location::current()
			);

		public:
			virtual COMError& operator=(const COMError& other);
			virtual COMError& operator=(COMError&& other) noexcept;

		public:
			virtual HRESULT GetHResult() const noexcept;
			virtual const char* what() const noexcept override;

		protected:
			virtual void GenerateErrorMessage(
				const std::source_location& location,
				const std::string& message
			) override;

		protected:
			HRESULT m_hresult;
			std::string m_errorString;
	};
}