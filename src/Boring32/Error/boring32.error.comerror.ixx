module;

#include <string>
#include <stdexcept>
#include <source_location>
#include <Windows.h>

export module boring32.error.comerror;

export namespace Boring32::Error
{
	class ComError : public std::runtime_error
	{
		public:
			virtual ~ComError();
			ComError(const std::source_location& location, const std::string& msg, const HRESULT hr);

		public:
			virtual HRESULT GetHResult() const noexcept;
			virtual const char* what() const noexcept override;

		protected:
			HRESULT m_hresult;
			std::string m_errorString;
	};
}