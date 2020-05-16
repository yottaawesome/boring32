#pragma once
#include <string>
#include <stdexcept>
#include <Windows.h>

namespace Boring32::Error
{
	class Win32Exception : public std::runtime_error
	{
		public:
			virtual ~Win32Exception();
			Win32Exception(const char* msg);
			Win32Exception(const std::string& msg);
			Win32Exception(const char* msg, DWORD errorCode);
			Win32Exception(const std::string& msg, DWORD errorCode);

			virtual DWORD GetErrorCode() const;
			virtual std::wstring GetErrorCodeWString() const;
			virtual std::wstring GetFullErrorWString() const;

		protected:
			const DWORD m_errorCode;
	};

	std::wstring GetErrorCodeWString(const DWORD errorCode);
}
