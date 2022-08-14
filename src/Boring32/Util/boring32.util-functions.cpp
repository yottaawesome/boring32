module;

#include <stdexcept>
#include <string>
#include <chrono>
#include <source_location>
#include <Windows.h>
#include <comdef.h>
#include <pathcch.h>
#include <Objbase.h>
#include <netlistmgr.h>
#include <wrl/client.h>

module boring32.util:functions;
import boring32.error;

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
                throw Error::Win32Error("GetModuleFileNameW() failed", GetLastError());
        }

        const HRESULT result = PathCchRemoveFileSpec(&filePath[0], filePath.size());
        if (result != S_OK && result != S_FALSE)
            throw Error::COMError("PathCchRemoveFileSpec() failed", result);
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
            throw Error::Win32Error("FileTimeToSystemTime() failed", GetLastError());
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

    // Adapted from https://stackoverflow.com/a/19941516/7448661
    std::wstring GetGuidAsWString(const GUID& guid)
    {
        std::wstring rawGuid(64, '\0');
        int numberOfChars = StringFromGUID2(guid, &rawGuid[0], 64);
        if (numberOfChars == 0)
            throw Error::Boring32Error("StringFromGUID2() failed");
        rawGuid.resize(numberOfChars - 1); // remove null terminator
        return rawGuid;
    }

    std::wstring GetGuidAsWString()
    {
        GUID guidReference;
        HRESULT result = CoCreateGuid(&guidReference);
        if (FAILED(result))
            throw Error::COMError("CoCreateGuid() failed", result);
        return GetGuidAsWString(guidReference);
    }

    GUID GenerateGUID()
    {
        GUID guid;
        HRESULT result = CoCreateGuid(&guid);
        if (FAILED(result))
            throw Error::COMError("CoCreateGuid() failed", result);
        return guid;
    }

    // TODO: find a better home for this function
    bool IsConnectedToInternet()
    {
        // https://docs.microsoft.com/en-us/windows/win32/api/netlistmgr/nn-netlistmgr-inetworklistmanager
        Microsoft::WRL::ComPtr<INetworkListManager> networkListManager;
        // https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-cocreateinstance
        // https://docs.microsoft.com/en-us/windows/win32/learnwin32/creating-an-object-in-com
        HRESULT result = CoCreateInstance(
            CLSID_NetworkListManager,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_INetworkListManager,
            reinterpret_cast<void**>(networkListManager.GetAddressOf())
        );
        if (FAILED(result))
            throw Error::COMError("CoCreateGuid() failed", result);

        // https://docs.microsoft.com/en-us/windows/win32/api/netlistmgr/ne-netlistmgr-nlm_connectivity
        NLM_CONNECTIVITY connectivity;
        result = networkListManager->GetConnectivity(&connectivity);
        if (FAILED(result))
            throw Error::COMError("GetConnectivity() failed", result);

        if (connectivity & NLM_CONNECTIVITY::NLM_CONNECTIVITY_IPV4_INTERNET)
            return true;
        if (connectivity & NLM_CONNECTIVITY::NLM_CONNECTIVITY_IPV6_INTERNET)
            return true;
        return false;
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
