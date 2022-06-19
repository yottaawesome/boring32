module;

#include <iostream>
#include <format>
#include <Windows.h>
#include <wlanapi.h>
#include <wtypes.h>

export module boring32.wirelesslan:cleanup;

namespace Boring32::WirelessLAN
{
    void CloseWLANHandle(HANDLE handle)
    {
        // https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlanclosehandle
        if (DWORD status = WlanCloseHandle(handle, 0) != ERROR_SUCCESS)
            std::wcerr << std::format(L"WlanCloseHandle() failed: {}\n", status);
    }

    struct HandleDeleter
    {
        void operator()(HANDLE handle)
        {
            CloseWLANHandle(handle);
        }
    };
    using UniqueWLANHandle = std::unique_ptr<std::remove_pointer<HANDLE>::type, HandleDeleter>;
    using SharedWLANHandle = std::shared_ptr<std::remove_pointer<HANDLE>::type>;
    SharedWLANHandle CreateSharedWLANHandle(HANDLE handle)
    {
        return { handle, CloseWLANHandle };
    }

    struct MemoryDeleter
    {
        void operator()(void* handle)
        {
            // https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlanfreememory
            WlanFreeMemory(handle);
        }
    };
    using UniqueWLANMemory = std::unique_ptr<void, MemoryDeleter>;
    using SharedWLANMemory = std::shared_ptr<void>;
    SharedWLANMemory CreateSharedWLANMemory(void* memory)
    {
        return { memory, WlanFreeMemory };
    }
}