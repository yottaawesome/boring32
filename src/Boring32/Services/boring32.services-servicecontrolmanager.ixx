export module boring32.services:servicecontrolmanager;
import boring32.shared;
import boring32.error;
import :service;
import :raii;

export namespace Boring32::Services
{
	class ServiceControlManager final
	{
		public:
			~ServiceControlManager() = default;
			ServiceControlManager(const ServiceControlManager&) = default;
			ServiceControlManager& operator=(const ServiceControlManager&) = default;
			ServiceControlManager(ServiceControlManager&&) noexcept = default;
			ServiceControlManager& operator=(ServiceControlManager&&) noexcept = default;

		public:
			ServiceControlManager()
			{
				Open(Win32::_SC_MANAGER_ALL_ACCESS);
			}

			ServiceControlManager(const unsigned desiredAccess)
			{
				Open(desiredAccess);
			}

		public:
			operator bool() const noexcept
			{
				return m_scm.get() != nullptr;
			}

		public:
			void Close()
			{
				m_scm = nullptr;
			}

			Service AccessService(
				const std::wstring& name
			)
			{
				return AccessService(name, Win32::_SERVICE_ALL_ACCESS);
			}

			Service AccessService(
				const std::wstring& name,
				const unsigned desiredAccess
			)
			{
				if (!m_scm)
					throw Error::Boring32Error("m_scm is null");

				// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-openservicew
				Win32::SC_HANDLE serviceHandle = Win32::OpenServiceW(
					m_scm.get(),
					name.c_str(),
					desiredAccess
				);
				if (!serviceHandle)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("OpenServiceW() failed", lastError);
				}
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
				if (!schSCManager)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("OpenSCManagerW() failed", lastError);
				}
				m_scm = CreateSharedPtr(schSCManager);
			}

		private:
			ServiceHandleSharedPtr m_scm;
	};
}
