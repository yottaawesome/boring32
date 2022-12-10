export module boring32.services:functions;
import <string>;
import <win32.hpp>;

export namespace Boring32::Services
{
	[[nodiscard]] SC_HANDLE OpenServiceControlManager(const DWORD desiredAccess);
	[[nodiscard]] SERVICE_STATUS_PROCESS GetServiceStatus(const SC_HANDLE serviceHandle);
	[[nodiscard]] SC_HANDLE OpenServiceHandle(
		const SC_HANDLE scmHandle, 
		const std::wstring& serviceName, 
		const DWORD desiredAccess
	);
}
