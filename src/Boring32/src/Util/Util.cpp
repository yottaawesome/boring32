module;

#include <stdexcept>
#include <string>
#include <chrono>
#include <source_location>
#include <Windows.h>
#include <comdef.h>
#include <pathcch.h>

module boring32.util;
import boring32.error.win32error;
import boring32.error.comerror;

namespace Boring32::Util
{
    std::wstring GetCurrentExecutableDirectory()
    {
        constexpr size_t blockSize = 2048;
        std::wstring filePath(L"\0", 0);
        DWORD status = ERROR_INSUFFICIENT_BUFFER;
        while (status == ERROR_INSUFFICIENT_BUFFER)
        {
            filePath.resize(filePath.size() + blockSize);
            status = GetModuleFileNameW(nullptr, &filePath[0], (DWORD)filePath.size());
            if(!status)
                throw Error::Win32Error(__FUNCSIG__ ": GetModuleFileNameW() failed", GetLastError());
        }

        const HRESULT result = PathCchRemoveFileSpec(&filePath[0], filePath.size());
        if (result != S_OK && result != S_FALSE)
            throw Error::ComError(std::source_location::current(), "PathCchRemoveFileSpec() failed", result);
        filePath = filePath.c_str();
        
        return filePath;
    }

    SYSTEMTIME LargeIntegerTimeToSystemTime(const LARGE_INTEGER& li)
    {
        FILETIME ft{ 0 };
        ft.dwLowDateTime = li.LowPart;
        ft.dwHighDateTime = li.HighPart;
        SYSTEMTIME st{ 0 };
        if (FileTimeToSystemTime(&ft, &st) == false)
            throw Error::Win32Error(__FUNCSIG__ ": FileTimeToSystemTime() failed", GetLastError());
        return st;
    }

    size_t GetUnixTime() noexcept
    {
        const auto rightNow = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::seconds>(rightNow.time_since_epoch()).count();
    }

    size_t GetMillisToMinuteBoundary(const SYSTEMTIME& time, const size_t minuteBoundary) noexcept
    {
        size_t minutesToMillis = static_cast<size_t>(time.wMinute) * 60 * 1000;
        minutesToMillis += static_cast<size_t>(time.wSecond) * 1000;
        minutesToMillis += time.wMilliseconds;
        size_t boundaryToMillis = minuteBoundary * 60 * 1000;
        return boundaryToMillis - (minutesToMillis % boundaryToMillis);
    }

    size_t GetMillisToSecondBoundary(const SYSTEMTIME& time, const size_t secondBoundary) noexcept
    {
        size_t currentSecondMillis = static_cast<size_t>(time.wSecond) * 1000;
        currentSecondMillis += time.wMilliseconds;
        size_t boundaryMillis = secondBoundary * 1000;
        return boundaryMillis - (currentSecondMillis % boundaryMillis);
    }

    

    /*std::vector<std::byte> StringToByteVector(const std::wstring_view str)
    {
        return { 
            reinterpret_cast<const std::byte*>(&str[0]), 
            reinterpret_cast<const std::byte*>(&str[0]) + str.size() * sizeof(wchar_t) 
        };
    }

    std::vector<std::byte> StringToByteVector(const std::string_view str)
    {
        return { 
            reinterpret_cast<const std::byte*>(&str[0]), 
            reinterpret_cast<const std::byte*>(&str[0]) + str.size() * sizeof(char)
        };
    }*/

    /*template<typename X>
    X Blah()
    {
        return X();
    }*/

}
