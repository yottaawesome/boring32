export module boring32.services:servicecontrolmanager;
import std;

import <win32.hpp>;
import :raii;
import :service;
import boring32.error;

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
				Open(SC_MANAGER_ALL_ACCESS);
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
				return AccessService(name, SERVICE_ALL_ACCESS);
			}

			Service AccessService(
				const std::wstring& name,
				const unsigned desiredAccess
			)
			{
				if (!m_scm)
					throw Error::Boring32Error("m_scm is null");

				// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-openservicew
				SC_HANDLE serviceHandle = OpenServiceW(
					m_scm.get(),
					name.c_str(),
					desiredAccess
				);
				if (!serviceHandle)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("OpenServiceW() failed", lastError);
				}
				return { CreateSharedPtr(serviceHandle) };
			}

			SC_HANDLE GetHandle() const noexcept
			{
				return m_scm.get();
			}

		private:
			void Open(const unsigned desiredAccess)
			{
				// https://docs.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-openscmanagerw
				SC_HANDLE schSCManager = OpenSCManagerW(
					nullptr,        // local computer
					nullptr,        // ServicesActive database 
					desiredAccess   // full access rights
				);
				if (!schSCManager)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("OpenSCManagerW() failed", lastError);
				}
				m_scm = CreateSharedPtr(schSCManager);
			}

		private:
			ServiceHandleSharedPtr m_scm;
	};
}
