#pragma once
#include <string>
#include <stdexcept>
#include <Windows.h>

namespace Boring32::Error
{
	class Win32Error : public std::runtime_error
	{
		public:
			virtual ~Win32Error();
			Win32Error(const char* msg, const DWORD errorCode);
			Win32Error(const std::string& msg, const DWORD errorCode);

			virtual DWORD GetErrorCode() const noexcept;
			virtual const char* what() const noexcept override;

		protected:
			DWORD m_errorCode;
			std::string m_errorString;
	};
}