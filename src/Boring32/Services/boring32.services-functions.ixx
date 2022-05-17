module;

#include <string>
#include <Windows.h>

export module boring32.services:functions;

export namespace Boring32::Services
{
	[[nodiscard]] SC_HANDLE OpenServiceControlManager(const DWORD desiredAccess);
	[[nodiscard]] SERVICE_STATUS_PROCESS GetServiceStatus();
	[[nodiscard]] SC_HANDLE OpenServiceHandle(
		const SC_HANDLE scmHandle, 
		const std::wstring& serviceName, 
		const DWORD desiredAccess
	);
}
