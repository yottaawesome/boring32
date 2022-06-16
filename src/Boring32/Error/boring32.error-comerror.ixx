module;

#include <string>
#include <stdexcept>
#include <source_location>
#include <Windows.h>

export module boring32.error:comerror;
import :boring32error;

export namespace Boring32::Error
{
	class ComError : public Boring32Error
	{
		public:
			virtual ~ComError();
			ComError(const ComError& other);
			ComError(ComError&& other) noexcept;
			ComError(
				const std::string& msg, 
				const HRESULT hr,
				const std::source_location location = std::source_location::current()
			);

		public:
			virtual ComError& operator=(const ComError& other);
			virtual ComError& operator=(ComError&& other) noexcept;

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