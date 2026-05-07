export module boring32:services.servicecontrolmanager;
import std;
import :win32;
import :error;
import :services.service;
import :services.raii;

export namespace Boring32::Services
{
	class ServiceControlManager final
	{
	public:
		ServiceControlManager(const ServiceControlManager&) = delete;
		ServiceControlManager& operator=(const ServiceControlManager&) = delete;
		ServiceControlManager(ServiceControlManager&&) noexcept = default;
		ServiceControlManager& operator=(ServiceControlManager&&) noexcept = default;

		ServiceControlManager()
		{
			Open(Win32::ScManagerAllAccess);
		}

		ServiceControlManager(unsigned desiredAccess)
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

		[[nodiscard]]
		auto AccessService(const std::wstring& name) -> Service
		{
			return AccessService(name, Win32::ServiceAllAccess);
		}

		[[nodiscard]]
		auto AccessService(const std::wstring& name, unsigned desiredAccess) -> Service
		{
			if (not m_scm)
				throw Error::Boring32Error{"m_scm is null"};
			// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-openservicew
			auto serviceHandle = Win32::OpenServiceW(m_scm.get(), name.c_str(), desiredAccess);
			if (not serviceHandle)
				throw Error::Win32Error{Win32::GetLastError(), "OpenServiceW() failed"};
			return ServiceHandleUniquePtr{ serviceHandle };
		}

		[[nodiscard]]
		auto GetHandle() const noexcept -> Win32::SC_HANDLE
		{
			return m_scm.get();
		}

	private:
		void Open(unsigned desiredAccess)
		{
			// https://docs.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-openscmanagerw
			auto schSCManager = Win32::OpenSCManagerW(nullptr, nullptr, desiredAccess);
			if (not schSCManager)
				throw Error::Win32Error{Win32::GetLastError(), "OpenSCManagerW() failed"};
			m_scm = ServiceHandleUniquePtr(schSCManager);
		}

		ServiceHandleUniquePtr m_scm;
	};
}
