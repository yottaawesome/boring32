export module boring32:services_functions;
import std;
import boring32.win32;
import :error;
import :services_raii;

export namespace Boring32::Services
{
	[[nodiscard]]
	auto OpenServiceControlManager(Win32::DWORD desiredAccess) -> Win32::SC_HANDLE
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-openscmanagerw
		// https://docs.microsoft.com/en-us/windows/win32/services/service-security-and-access-rights
		SC_HANDLE scmHandle = Win32::OpenSCManagerW(nullptr, Win32::_SERVICES_ACTIVE_DATABASE, desiredAccess);
		return scmHandle ? scmHandle : throw Error::Win32Error(Win32::GetLastError(), "OpenSCManagerW() failed");
	}

	[[nodiscard]]
	auto GetServiceStatus(Win32::SC_HANDLE serviceHandle) -> Win32::SERVICE_STATUS_PROCESS
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

	[[nodiscard]]
	auto ServiceExists(const std::wstring& serviceName) -> bool
	{
		if (serviceName.empty())
			return false;

		Win32::SC_HANDLE scmHandle = OpenServiceControlManager(Win32::GenericRead);
		if (not scmHandle)
			throw Error::Win32Error(Win32::GetLastError(), "OpenServiceControlManager() failed");
		ServiceHandleUniquePtr scm(scmHandle);

		Win32::SC_HANDLE serviceHandle = Win32::OpenServiceW(
			scmHandle,
			serviceName.c_str(),
			Win32::GenericRead // https://docs.microsoft.com/en-us/windows/win32/services/service-security-and-access-rights
		);
		ServiceHandleUniquePtr service(serviceHandle);

		return serviceHandle;
	}

	[[nodiscard]] 
	auto OpenServiceHandle(
		Win32::SC_HANDLE scmHandle,
		const std::wstring& serviceName, 
		Win32::DWORD desiredAccess
	) -> Win32::SC_HANDLE
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

	[[nodiscard]]
	auto OpenServiceHandle(
		Win32::SC_HANDLE scmHandle,
		const std::wstring& serviceName,
		Win32::DWORD desiredAccess,
		const std::nothrow_t&
	) noexcept -> std::optional<Win32::SC_HANDLE>
	try
	{
		return OpenServiceHandle(scmHandle, serviceName, desiredAccess);
	}
	catch (...)
	{
		return std::nullopt;
	}
}
