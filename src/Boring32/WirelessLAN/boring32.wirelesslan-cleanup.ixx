export module boring32.wirelesslan:cleanup;
import <iostream>;
import <format>;
import <memory>;
import boring32.win32;

namespace Boring32::WirelessLAN
{
    void CloseWLANHandle(Win32::HANDLE handle)
    {
        // https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlanclosehandle
        if (Win32::DWORD status = Win32::WlanCloseHandle(handle, 0) != Win32::ErrorCodes::Success)
            std::wcerr << std::format(L"WlanCloseHandle() failed: {}\n", status);
    }

    struct HandleDeleter
    {
        void operator()(HANDLE handle)
        {
            CloseWLANHandle(handle);
        }
    };
    using UniqueWLANHandle = std::unique_ptr<std::remove_pointer<Win32::HANDLE>::type, HandleDeleter>;
    using SharedWLANHandle = std::shared_ptr<std::remove_pointer<Win32::HANDLE>::type>;
    SharedWLANHandle CreateSharedWLANHandle(HANDLE handle)
    {
        return { handle, CloseWLANHandle };
    }

    struct MemoryDeleter
    {
        void operator()(void* handle)
        {
            // https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlanfreememory
            Win32::WlanFreeMemory(handle);
        }
    };
    using UniqueWLANMemory = std::unique_ptr<void, MemoryDeleter>;
    using SharedWLANMemory = std::shared_ptr<void>;
    SharedWLANMemory CreateSharedWLANMemory(void* memory)
    {
        return { memory, Win32::WlanFreeMemory };
    }
}