module;

#include <format>
#include <iostream>
#include <source_location>
#include <Windows.h>
#include <wlanapi.h>
#include <wtypes.h>

module boring32.wirelesslan:session;
import boring32.error;

namespace Boring32::WirelessLAN
{
	Session::~Session() {}

	Session::Session()
        : m_maxClientVersion(2),
        m_negotiatedVersion(0)
	{
        Open();
	}

    void Session::Open()
    {
        // Open a WLAN session
        HANDLE wlanHandle;
        DWORD status = WlanOpenHandle(
            m_maxClientVersion,
            nullptr,
            &m_negotiatedVersion,
            &wlanHandle
        );
        if (status != ERROR_SUCCESS)
            throw Boring32::Error::Win32Error("WlanOpenHandle() failed", status);
        m_wlanHandle = CreateSharedWLANHandle(wlanHandle);
    }

    DWORD Session::GetMaxClientVersion() const noexcept
    {
        return m_maxClientVersion;
    }

    DWORD Session::GetNegotiatedVersion() const noexcept
    {
        return m_negotiatedVersion;
    }
    
    void Session::Close()
    {
        m_wlanHandle = nullptr;
        m_maxClientVersion = 0;
        m_negotiatedVersion = 0;
    }
}