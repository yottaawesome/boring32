module;

#include <string>
#include <stdexcept>
#include <source_location>
#include <Windows.h>

export module boring32.error:win32error;

export namespace Boring32::Error
{
	class Win32Error : public std::runtime_error
	{
		public:
			virtual ~Win32Error();
			Win32Error(const Win32Error& other);
			Win32Error(Win32Error&& other) noexcept;
			Win32Error(
				const std::string& msg,
				const std::source_location location = std::source_location::current()
			);
			Win32Error(
				const std::string& msg,
				const DWORD errorCode,
				const std::source_location location = std::source_location::current()
			);
			Win32Error(
				const std::string& msg, 
				const DWORD errorCode, 
				const std::wstring& moduleName,
				const std::source_location location = std::source_location::current()
			);

		public:
			virtual Win32Error& operator=(const Win32Error& other);
			virtual Win32Error& operator=(Win32Error&& other) noexcept;

		public:
			virtual DWORD GetErrorCode() const noexcept;
			virtual const char* what() const noexcept override;

		protected:
			DWORD m_errorCode;
			std::string m_errorString;
	};
}