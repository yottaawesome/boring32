module;

#include <Windows.h>

export module boring32.services:functions;

export namespace Boring32::Services
{
	SC_HANDLE OpenServiceControlManager(const DWORD desiredAccess);
	SERVICE_STATUS_PROCESS GetServiceStatus();
}
