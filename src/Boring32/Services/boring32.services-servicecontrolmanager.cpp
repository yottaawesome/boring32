module;

#include <source_location>
#include <Windows.h>

module boring32.services:servicecontrolmanager;
import boring32.error;

namespace Boring32::Services
{
	ServiceControlManager::~ServiceControlManager(){}

	ServiceControlManager::ServiceControlManager()
	{
		Open();
	}

	void ServiceControlManager::Open()
	{
        SC_HANDLE schSCManager;

        // Get a handle to the SCM database. 

        schSCManager = OpenSCManagerW(
            nullptr,                // local computer
            nullptr,                // ServicesActive database 
            SC_MANAGER_ALL_ACCESS   // full access rights
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
}