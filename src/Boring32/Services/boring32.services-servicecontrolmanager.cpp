module;

#include <string>
#include <source_location>
#include <Windows.h>

module boring32.services:servicecontrolmanager;
import boring32.error;

namespace Boring32::Services
{
	ServiceControlManager::~ServiceControlManager(){}

	ServiceControlManager::ServiceControlManager()
	{
		Open(SC_MANAGER_ALL_ACCESS);
	}

    ServiceControlManager::ServiceControlManager(const unsigned desiredAccess)
    {
        Open(desiredAccess);
    }

	void ServiceControlManager::Open(const unsigned desiredAccess)
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

    void ServiceControlManager::Close()
    {
        m_scm = nullptr;
    }

    Service ServiceControlManager::AccessService(const std::wstring& name)
    {
        return AccessService(name, SERVICE_ALL_ACCESS);
    }

    SC_HANDLE ServiceControlManager::GetHandle() const noexcept
    {
        return m_scm.get();
    }

    Service ServiceControlManager::AccessService(
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
        return { CreateSharedPtr (serviceHandle)};
    }
    
    ServiceControlManager::operator bool() const noexcept
    {
        return m_scm.get() != nullptr;
    }
}