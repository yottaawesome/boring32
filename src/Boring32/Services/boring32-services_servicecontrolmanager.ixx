export module boring32:services_servicecontrolmanager;
import std;
import boring32.win32;
import :error;
import :services_service;
import :services_raii;

export namespace Boring32::Services
{
	struct ServiceControlManager final
	{
		ServiceControlManager(const ServiceControlManager&) = default;
		ServiceControlManager& operator=(const ServiceControlManager&) = default;
		ServiceControlManager(ServiceControlManager&&) noexcept = default;
		ServiceControlManager& operator=(ServiceControlManager&&) noexcept = default;

		ServiceControlManager()
		{
			Open(Win32::_SC_MANAGER_ALL_ACCESS);
		}

		ServiceControlManager(const unsigned desiredAccess)
		{
			Open(desiredAccess);
		}

		operator bool() const noexcept
		{
			return m_scm.get() != nullptr;
		}

		void Close()
		{
			m_scm = nullptr;
		}

		Service AccessService(const std::wstring& name)
		{
			return AccessService(name, Win32::_SERVICE_ALL_ACCESS);
		}

		Service AccessService(const std::wstring& name, unsigned desiredAccess)
		{
			if (not m_scm)
				throw Error::Boring32Error("m_scm is null");

			// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-openservicew
			Win32::SC_HANDLE serviceHandle = Win32::OpenServiceW(
				m_scm.get(),
				name.c_str(),
				desiredAccess
			);
			if (not serviceHandle)
				throw Error::Win32Error(Win32::GetLastError(), "OpenServiceW() failed");
			return { CreateSharedPtr(serviceHandle) };
		}

		Win32::SC_HANDLE GetHandle() const noexcept
		{
			return m_scm.get();
		}

		private:
		void Open(const unsigned desiredAccess)
		{
			// https://docs.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-openscmanagerw
			Win32::SC_HANDLE schSCManager = Win32::OpenSCManagerW(
				nullptr,        // local computer
				nullptr,        // ServicesActive database 
				desiredAccess   // full access rights
			);
			if (not schSCManager)
				throw Error::Win32Error(Win32::GetLastError(), "OpenSCManagerW() failed");
			m_scm = CreateSharedPtr(schSCManager);
		}

		ServiceHandleSharedPtr m_scm;
	};
}
