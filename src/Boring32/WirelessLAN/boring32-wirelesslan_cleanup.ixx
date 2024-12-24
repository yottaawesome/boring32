export module boring32:wirelesslan_cleanup;
import boring32.shared;
import :raii;

namespace Boring32::WirelessLAN
{
    void CloseWLANHandle(Win32::HANDLE handle) noexcept
    {
        // https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlanclosehandle
        Win32::WlanCloseHandle(handle, 0);
    }

    using UniqueWLANHandle = RAII::IndirectUniquePtr<Win32::HANDLE, CloseWLANHandle>;
    using SharedWLANHandle = std::shared_ptr<std::remove_pointer_t<Win32::HANDLE>>;
    SharedWLANHandle CreateSharedWLANHandle(HANDLE handle)
    {
        return { handle, CloseWLANHandle };
    }

    // https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlanfreememory
    using UniqueWLANMemory = RAII::UniquePtr<void, Win32::WlanFreeMemory>;
    using SharedWLANMemory = std::shared_ptr<void>;
    SharedWLANMemory CreateSharedWLANMemory(void* memory)
    {
        return { memory, Win32::WlanFreeMemory };
    }
}