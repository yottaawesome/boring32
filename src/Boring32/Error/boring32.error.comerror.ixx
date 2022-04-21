module;

#include <string>
#include <stdexcept>
#include <source_location>
#include <Windows.h>

export module boring32.error:comerror;

export namespace Boring32::Error
{
	class ComError : public std::runtime_error
	{
		public:
			virtual ~ComError();
			ComError(const ComError& other);
			ComError(ComError&& other) noexcept;
			ComError(
				const std::source_location& location, 
				const std::string& msg, 
				const HRESULT hr
			);

		public:
			virtual ComError& operator=(const ComError& other);
			virtual ComError& operator=(ComError&& other) noexcept;

		public:
			virtual HRESULT GetHResult() const noexcept;
			virtual const char* what() const noexcept override;

		protected:
			HRESULT m_hresult;
			std::string m_errorString;
	};
}