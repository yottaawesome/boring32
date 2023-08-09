export module boring32.services:functions;
import std;
import <win32.hpp>;

import boring32.error;

export namespace Boring32::Services
{
	[[nodiscard]] SC_HANDLE OpenServiceControlManager(const DWORD desiredAccess)
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-openscmanagerw
		const SC_HANDLE scmHandle = OpenSCManagerW(
			nullptr,
			SERVICES_ACTIVE_DATABASE,
			desiredAccess // https://docs.microsoft.com/en-us/windows/win32/services/service-security-and-access-rights
		);
		if (!scmHandle)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("OpenSCManagerW() failed", lastError);
		}

		return scmHandle;
	}

	[[nodiscard]] SERVICE_STATUS_PROCESS GetServiceStatus(const SC_HANDLE serviceHandle)
	{
		if (!serviceHandle)
			throw Boring32::Error::Boring32Error("Service handle cannot be null");

		DWORD bytesNeeded = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/winsvc/ns-winsvc-service_status_process
		SERVICE_STATUS_PROCESS serviceStatus{ 0 };
		// https://docs.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-queryservicestatusex
		const bool succeeded = QueryServiceStatusEx(
			serviceHandle,
			SC_STATUS_PROCESS_INFO, // this is the only one defined
			reinterpret_cast<BYTE*>(&serviceStatus),
			sizeof(serviceStatus),
			&bytesNeeded
		);
		if (!succeeded)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("QueryServiceStatusEx() failed", lastError);
		}

		return serviceStatus;
	}

	[[nodiscard]] SC_HANDLE OpenServiceHandle(
		const SC_HANDLE scmHandle, 
		const std::wstring& serviceName, 
		const DWORD desiredAccess
	)
	{
		if (!scmHandle)
			throw Boring32::Error::Boring32Error("SCM handle cannot be null");

		// https://docs.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-openservicew
		const SC_HANDLE serviceHandle = OpenServiceW(
			scmHandle,
			serviceName.c_str(),
			desiredAccess // https://docs.microsoft.com/en-us/windows/win32/services/service-security-and-access-rights
		);
		if (!serviceHandle)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("OpenServiceW() failed", lastError);
		}

		return serviceHandle;
	}
}
