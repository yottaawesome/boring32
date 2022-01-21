module;

#include <string>
#include <source_location>
#include <Windows.h>
#include <winhttp.h>

module boring32.winhttp.winhttperror;

namespace Boring32::WinHttp
{
	WinHttpError::~WinHttpError() = default;
	
	WinHttpError::WinHttpError(
		const std::source_location& location, 
		const std::string& msg, 
		const DWORD errorCode
	)
	:	Error::Win32Error(
		location, 
		msg, 
		errorCode,
		errorCode >= WINHTTP_ERROR_BASE && errorCode <= WINHTTP_ERROR_LAST ? L"winhttp.dll" : L""
	)
	{ }
}