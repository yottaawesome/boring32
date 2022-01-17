module;

#include <string>
#include <stdexcept>
#include <source_location>
#include <Windows.h>

export module boring32.error.win32error;

export namespace Boring32::Error
{
	class Win32Error : public std::runtime_error
	{
		public:
			virtual ~Win32Error();
			Win32Error(const std::source_location& location, const char* msg);
			Win32Error(const char* msg, const DWORD errorCode);
			Win32Error(const std::string& msg, const DWORD errorCode);
			Win32Error(const std::source_location& location, const std::string& msg, const DWORD errorCode);

		public:
			virtual DWORD GetErrorCode() const noexcept;
			virtual const char* what() const noexcept override;

		protected:
			DWORD m_errorCode;
			std::string m_errorString;
	};
}