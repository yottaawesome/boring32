module;

#include <Windows.h>

export module boring32.services:functions;

export namespace Boring32::Services
{
	[[nodiscard]] SC_HANDLE OpenServiceControlManager(const DWORD desiredAccess);
	[[nodiscard]] SERVICE_STATUS_PROCESS GetServiceStatus();
}
