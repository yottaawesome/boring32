module;

#include <stdexcept>
#include <string>
#include <source_location>
#include <Windows.h>
#include <shlwapi.h>

module boring32.registry.functions;
import boring32.error.win32error;
import boring32.async.event;

namespace Boring32::Registry
{
    template<>
    std::wstring GetValue<std::wstring, RRF_RT_REG_SZ>(
        const HKEY key,
        const std::wstring& valueName
        )
    {
        if (key == nullptr)
            throw std::runtime_error(__FUNCSIG__ ": m_key is null");

        std::wstring out;
        DWORD sizeInBytes = 0;
        // https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-reggetvaluew
        LONG statusCode = RegGetValueW(
            key,
            nullptr,
            valueName.c_str(),
            RRF_RT_REG_SZ,
            nullptr,
            nullptr,
            &sizeInBytes
        );
        if (statusCode != ERROR_SUCCESS)
            throw Error::Win32Error(
                __FUNCSIG__ ": RegGetValueW() failed (1)",
                statusCode
            );

        out.resize(sizeInBytes / sizeof(wchar_t), '\0');
        statusCode = RegGetValueW(
            key,
            nullptr,
            valueName.c_str(),
            RRF_RT_REG_SZ,
            nullptr,
            &out[0],
            &sizeInBytes
        );
        if (statusCode != ERROR_SUCCESS)
            throw Error::Win32Error(
                __FUNCSIG__ ": RegGetValueW() failed (2)",
                statusCode
            );

        out.resize(sizeInBytes / sizeof(wchar_t));
        // Exclude terminating null
        if (out.empty() == false)
            out.pop_back();
        return out;
    }

    void GetValue(
        const HKEY key,
        const std::wstring& valueName,
        std::wstring& out
    )
    {
        out = GetValue<std::wstring, RRF_RT_REG_SZ>(key, valueName);
    }

    void GetValue(
        const HKEY key,
        const std::wstring& valueName,
        DWORD& out
    )
    {
        out = GetValue<DWORD, RRF_RT_REG_DWORD>(key, valueName);
    }

    void GetValue(
        const HKEY key,
        const std::wstring& valueName,
        size_t& out
    )
    {
        out = GetValue<DWORD, RRF_RT_REG_QWORD>(key, valueName);
    }

    // This should probably be integrated into RegistryKey to enable safety checks
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
            throw Error::Win32Error(std::source_location::current(), "failed to watch registry key for changes", status);
    }

    void DeleteKeyAndSubkey(const HKEY parent, const std::wstring& subkey)
    {
        if (parent == nullptr)
            throw std::invalid_argument(__FUNCSIG__ ": parent cannot be nullptr");

        // https://docs.microsoft.com/en-us/windows/win32/api/shlwapi/nf-shlwapi-shdeletekeyw
        const LSTATUS status = SHDeleteKeyW(
            parent,
            subkey.c_str()
        );
        if (status != ERROR_SUCCESS)
            throw Error::Win32Error(std::source_location::current(), "SHDeleteKeyW() failed", GetLastError());
    }

    void DeleteSubkeys(const HKEY parent, const std::wstring& subkey)
    {
        if (parent == nullptr)
            throw std::invalid_argument(__FUNCSIG__ ": parent cannot be nullptr");

        // https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regdeletetreew
        const LSTATUS status = RegDeleteTreeW(
            parent,
            subkey.c_str()
        );
        if (status != ERROR_SUCCESS)
            throw Error::Win32Error(std::source_location::current(), "RegDeleteTreeW() failed", GetLastError());
    }
}