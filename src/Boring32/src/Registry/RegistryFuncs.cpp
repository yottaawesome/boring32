#include "pch.hpp"
#include "include/Async/Event.hpp"
#include "include/Error/Win32Error.hpp"
#include "include/Registry/Registry.hpp"

namespace Boring32::WindowsRegistry
{
    // This should probably be integrated into RegKey to enable safety checks
    // against calling it multiple times causing resource leaks
    void WatchKey(const HKEY key, const Async::Event& eventToSignal)
    {
        if (key == nullptr)
            throw std::invalid_argument(__FUNCSIG__ ": key cannot be nullptr");
        if (eventToSignal.GetHandle() == nullptr)
            throw std::invalid_argument(__FUNCSIG__ ": eventToSignal is not an initialised Event");

        // https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regnotifychangekeyvalue
        LSTATUS status = RegNotifyChangeKeyValue(
            key,
            false,
            REG_NOTIFY_CHANGE_LAST_SET,
            eventToSignal.GetHandle(),
            true
        );
        if (status != ERROR_SUCCESS)
            throw Error::Win32Error(__FUNCSIG__ ": failed to watch registry key for changes", status);
    }
}