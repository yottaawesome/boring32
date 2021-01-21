#include "pch.hpp"
#include <stdexcept>
#include <chrono>
#include "include/Error/Win32Error.hpp"
#include "include/Error/ComError.hpp"
#include "include/Util/Util.hpp"

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
            if(status == 0)
                throw Error::Win32Error(__FUNCSIG__ ": GetModuleFileNameW() failed", GetLastError());
        }

        HRESULT result = PathCchRemoveFileSpec(&filePath[0], filePath.size());
        if (result != S_OK && result != S_FALSE)
            throw Error::ComError(__FUNCSIG__ ": PathCchRemoveFileSpec() failed", result);
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

    size_t GetUnixTime()
    {
        const auto rightNow = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::seconds>(rightNow.time_since_epoch()).count();
    }

    DWORD GetMillisToMinuteBoundary(const SYSTEMTIME& time, const UINT boundary)
    {
        int minuteModResult = time.wSecond > 0
            ? (time.wMinute + 1) % boundary
            : time.wMinute % boundary;
        DWORD millis = 0;
        if (minuteModResult > 0)
            millis += (boundary - minuteModResult) * 60000;
        if (time.wSecond > 0)
            millis += (60 - time.wSecond) * 1000;
        if (time.wMilliseconds > 0)
            millis -= 1000 - time.wMilliseconds;
        return millis;
    }
}
