#pragma once
#include <string>
#include <stdexcept>
#include <Windows.h>

namespace Boring32::Error
{
	class ComError : public std::runtime_error
	{
		public:
			virtual ~ComError();
			ComError(const char* msg, const HRESULT hr);
			ComError(const std::string& msg, const HRESULT hr);

			virtual HRESULT GetHResult() const noexcept;
			virtual const char* what() const noexcept override;

		protected:
			HRESULT m_hresult;
			std::string m_errorString;
	};
}