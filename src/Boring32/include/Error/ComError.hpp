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
			ComError(const char* msg, const HRESULT errorCode);
			ComError(const std::string& msg, const HRESULT errorCode);

			virtual HRESULT GetErrorCode() const noexcept;
			virtual const char* what() const noexcept override;

		protected:
			HRESULT m_errorCode;
			std::string m_errorString;
	};
}