export module boring32:services_functions;
import boring32.shared;
import :error;

export namespace Boring32::Services
{
	[[nodiscard]] Win32::SC_HANDLE OpenServiceControlManager(const Win32::DWORD desiredAccess)
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-openscmanagerw
		// https://docs.microsoft.com/en-us/windows/win32/services/service-security-and-access-rights
		SC_HANDLE scmHandle = Win32::OpenSCManagerW(nullptr, Win32::_SERVICES_ACTIVE_DATABASE, desiredAccess);
		if (not scmHandle)
			throw Error::Win32Error(Win32::GetLastError(), "OpenSCManagerW() failed");

		return scmHandle;
	}

	[[nodiscard]] Win32::SERVICE_STATUS_PROCESS GetServiceStatus(const Win32::SC_HANDLE serviceHandle)
	{
		if (not serviceHandle)
			throw Boring32::Error::Boring32Error("Service handle cannot be null");

		Win32::DWORD bytesNeeded = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/winsvc/ns-winsvc-service_status_process
		Win32::SERVICE_STATUS_PROCESS serviceStatus{ 0 };
		// https://docs.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-queryservicestatusex
		bool succeeded = Win32::QueryServiceStatusEx(
			serviceHandle,
			Win32::SC_STATUS_TYPE::SC_STATUS_PROCESS_INFO, // this is the only one defined
			reinterpret_cast<BYTE*>(&serviceStatus),
			sizeof(serviceStatus),
			&bytesNeeded
		);
		if (not succeeded)
			throw Error::Win32Error(Win32::GetLastError(), "QueryServiceStatusEx() failed");

		return serviceStatus;
	}

	[[nodiscard]] Win32::SC_HANDLE OpenServiceHandle(
		const Win32::SC_HANDLE scmHandle,
		const std::wstring& serviceName, 
		const Win32::DWORD desiredAccess
	)
	{
		if (not scmHandle)
			throw Boring32::Error::Boring32Error("SCM handle cannot be null");

		// https://docs.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-openservicew
		Win32::SC_HANDLE serviceHandle = Win32::OpenServiceW(
			scmHandle,
			serviceName.c_str(),
			desiredAccess // https://docs.microsoft.com/en-us/windows/win32/services/service-security-and-access-rights
		);
		if (not serviceHandle)
			throw Error::Win32Error(Win32::GetLastError(), "OpenServiceW() failed");

		return serviceHandle;
	}
}
