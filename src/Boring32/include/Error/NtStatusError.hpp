#pragma once
#include <string>
#include <stdexcept>
#include <Windows.h>

namespace Boring32::Error
{
	class NtStatusError : public std::runtime_error
	{
		public:
			virtual ~NtStatusError();
			NtStatusError(const char* msg, const LONG errorCode);
			NtStatusError(const std::string& msg, const LONG errorCode);

			virtual LONG GetErrorCode() const noexcept;
			virtual const char* what() const noexcept override;

		protected:
			LONG m_errorCode;
			std::string m_errorString;
	};
}