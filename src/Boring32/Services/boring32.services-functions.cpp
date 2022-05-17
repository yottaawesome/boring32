module;

#include <format>
#include <source_location>
#include <Windows.h>

module boring32.services:functions;
import boring32.error;

namespace Boring32::Services
{
	SC_HANDLE OpenServiceControlManager(const DWORD desiredAccess)
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-openscmanagerw
		const SC_HANDLE handle = OpenSCManagerW(
			nullptr,
			SERVICES_ACTIVE_DATABASE,
			desiredAccess // https://docs.microsoft.com/en-us/windows/win32/services/service-security-and-access-rights
		);
		if (!handle)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error(std::source_location::current(), "OpenSCManagerW() failed", lastError);
		}

		return handle;
	}

	SERVICE_STATUS_PROCESS GetServiceStatus()
	{
		return { 0 };
	}

	SC_HANDLE OpenServiceHandle(
		const SC_HANDLE scmHandle,
		const std::wstring& serviceName,
		const DWORD desiredAccess
	)
	{
		if (!scmHandle)
			throw Boring32::Error::Boring32Error(std::source_location::current(), "SCM handle cannot be null");

		// https://docs.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-openservicew
		const SC_HANDLE serviceHandle = OpenServiceW(
			scmHandle,
			serviceName.c_str(),
			desiredAccess
		);
		if (!serviceHandle)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error(std::source_location::current(), "OpenServiceW() failed", lastError);
		}

		return serviceHandle;
	}
}