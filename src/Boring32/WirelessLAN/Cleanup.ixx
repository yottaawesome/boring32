export module boring32:wirelesslan.cleanup;
import std;
import :win32;
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
	auto CreateSharedWLANHandle(Win32::HANDLE handle) -> SharedWLANHandle
    {
        return { handle, CloseWLANHandle };
    }

    // https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlanfreememory
    using UniqueWLANMemory = RAII::UniquePtr<void, Win32::WlanFreeMemory>;
    using SharedWLANMemory = std::shared_ptr<void>;
	auto CreateSharedWLANMemory(void* memory) -> SharedWLANMemory
    {
        return { memory, Win32::WlanFreeMemory };
    }
}