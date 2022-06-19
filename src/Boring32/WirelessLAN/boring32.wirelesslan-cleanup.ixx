module;

#include <iostream>
#include <format>
#include <Windows.h>
#include <wlanapi.h>
#include <wtypes.h>

export module boring32.wirelesslan:cleanup;

namespace Boring32::WirelessLAN
{
    struct WlanHandleDeleter
    {
        void operator()(HANDLE handle)
        {
            // https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlanclosehandle
            if (DWORD status = WlanCloseHandle(handle, 0) != ERROR_SUCCESS)
                std::wcerr << std::format(L"WlanCloseHandle() failed: {}\n", status);
        }
    };
    using UniquePtrWlanHandle = std::unique_ptr<std::remove_pointer<HANDLE>::type, WlanHandleDeleter>;

    struct WlanMemoryDeleter
    {
        void operator()(void* handle)
        {
            // https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlanfreememory
            WlanFreeMemory(handle);
        }
    };
    using UniquePtrWlanMemory = std::unique_ptr<void, WlanMemoryDeleter>;
}