#include "pch.hpp"
#include <winhttp.h>
#include "include/Strings/Strings.hpp"
#include "include/Error/Error.hpp"
#include "Boring32/include/Library/Library.hpp"

// TODO: clean up
namespace Boring32::Error
{
    std::string GetNtStatusError(const std::string msg, const NTSTATUS errorCode)
    {
        // See the caveats for NTSTATUS with FormatMessage here
        // https://stackoverflow.com/questions/7915215/how-do-i-convert-a-win32-exception-code-to-a-string
        std::string stringToHoldMessage = "";

        DWORD flags =
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS |
            FORMAT_MESSAGE_FROM_HMODULE;
        HMODULE handle = LoadLibraryW(L"ntdll.dll");
        if (handle == nullptr)
            return "Could not load ntdll.dll";

        void* ptrMsgBuf = nullptr;
        FormatMessageA(
            flags,
            handle,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // TODO this is deprecated
            (LPSTR)&ptrMsgBuf,
            0,
            nullptr
        );

        if (ptrMsgBuf != nullptr)
        {
            stringToHoldMessage = (LPSTR)ptrMsgBuf;
            LocalFree(ptrMsgBuf);
        }
        else
        {
            stringToHoldMessage = __FUNCSIG__;
            stringToHoldMessage += " failed to translate Win32 error code: " + std::to_string(errorCode);
        }
        FreeLibrary(handle);
        return msg + ": " + stringToHoldMessage;
    }

    void GetErrorCodeString(const DWORD errorCode, std::string& stringToHoldMessage) noexcept
    {
        stringToHoldMessage = "";

        DWORD flags =
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS;
        HMODULE handle = nullptr;
        if (errorCode >= WINHTTP_ERROR_BASE && errorCode <= WINHTTP_ERROR_LAST)
        {
            //GetModuleHandleEx(0, TEXT("winhttp.dll"), &handle);
            handle = GetModuleHandleW(L"winhttp.dll");
            flags |= FORMAT_MESSAGE_FROM_HMODULE;
        }

        void* ptrMsgBuf = nullptr;
        FormatMessageA(
            flags,
            handle,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // TODO this is deprecated
            (LPSTR)&ptrMsgBuf,
            0,
            nullptr);

        if (ptrMsgBuf != nullptr)
        {
            stringToHoldMessage = (LPSTR)ptrMsgBuf;
            LocalFree(ptrMsgBuf);
        }
        else
        {
            stringToHoldMessage = __FUNCSIG__;
            stringToHoldMessage += " failed to translate Win32 error code: " + std::to_string(errorCode);
        }
    }

    void GetErrorCodeString(const DWORD errorCode, HMODULE moduleToReadFrom, std::string& stringToHoldMessage) noexcept
    {
        if (moduleToReadFrom == nullptr)
        {
            stringToHoldMessage = "moduleToReadFrom is null";
            return;
        }

        stringToHoldMessage = "";
        DWORD flags =
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS;

        void* ptrMsgBuf = nullptr;
        FormatMessageA(
            flags,
            moduleToReadFrom,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // TODO this is deprecated
            (LPSTR)&ptrMsgBuf,
            0,
            nullptr
        );

        if (ptrMsgBuf != nullptr)
        {
            stringToHoldMessage = (LPSTR)ptrMsgBuf;
            LocalFree(ptrMsgBuf);
        }
        else
        {
            std::stringstream wss;
            wss
                << __FUNCSIG__
                << " failed to translate Win32 error code: "
                << std::to_string(errorCode);
            stringToHoldMessage = wss.str();
        }
    }

    void GetErrorCodeString(const DWORD errorCode, HMODULE moduleToReadFrom, std::wstring& stringToHoldMessage) noexcept
    {
        if (moduleToReadFrom == nullptr)
        {
            stringToHoldMessage = L"moduleToReadFrom is null";
            return;
        }

        stringToHoldMessage = L"";
        DWORD flags =
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS;
        
        void* ptrMsgBuf = nullptr;
        FormatMessageW(
            flags,
            moduleToReadFrom,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // TODO this is deprecated
            (LPTSTR)&ptrMsgBuf,
            0,
            nullptr
        );

        if (ptrMsgBuf != nullptr)
        {
            stringToHoldMessage = (LPWSTR)ptrMsgBuf;
            LocalFree(ptrMsgBuf);
        }
        else
        {
            std::wstringstream wss;
            wss
                << __FUNCSIG__
                << L" failed to translate Win32 error code: "
                << std::to_wstring(errorCode);
            stringToHoldMessage = wss.str();
        }
    }

	void GetErrorCodeString(const DWORD errorCode, std::wstring& stringToHoldMessage) noexcept
	{
        stringToHoldMessage = L"";
        DWORD flags =
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS;
        HMODULE handle = nullptr;
        if (errorCode >= WINHTTP_ERROR_BASE && errorCode <= WINHTTP_ERROR_LAST)
        {
            //GetModuleHandleExW(0, L"winhttp.dll", &handle);
            handle = LoadLibrary(L"winhttp.dll");
            flags |= FORMAT_MESSAGE_FROM_HMODULE;
        }

        void* ptrMsgBuf = nullptr;
        FormatMessageW(
            flags,
            handle,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // TODO this is deprecated
            (LPTSTR)&ptrMsgBuf,
            0,
            nullptr
        );
        if (handle)
            FreeLibrary(handle);

        if (ptrMsgBuf != nullptr)
        {
            stringToHoldMessage = (LPWSTR)ptrMsgBuf;
            LocalFree(ptrMsgBuf);
        }
        else
        {
            std::wstringstream wss;
            wss 
                << __FUNCSIG__ 
                << L" failed to translate Win32 error code: " 
                << std::to_wstring(errorCode);
            stringToHoldMessage = wss.str();
        }
	}

    std::wstring CreateErrorStringFromCode(const std::wstring msg, HMODULE moduleToReadFrom, const DWORD errorCode) noexcept
    {
        std::wstring translatedErrorMessage;
        GetErrorCodeString(errorCode, moduleToReadFrom, translatedErrorMessage);
        std::wstringstream wss;
        wss << msg
            << std::endl
            << translatedErrorMessage
            << L" (win32/ntstatus code: "
            << std::to_wstring(errorCode)
            << L")";
        return wss.str();
    }

    std::wstring CreateErrorStringFromCode(const std::wstring msg, const DWORD errorCode) noexcept
    {
        std::wstring translatedErrorMessage;
        GetErrorCodeString(errorCode, translatedErrorMessage);
        std::wstringstream wss;
        wss << msg
            << std::endl
            << translatedErrorMessage
            << L" (win32 code: "
            << std::to_wstring(errorCode)
            << L")";
        return wss.str();
    }

    std::string CreateErrorStringFromCode(const std::string msg, HMODULE moduleToReadFrom, const DWORD errorCode) noexcept
    {
        std::string translatedErrorMessage;
        GetErrorCodeString(errorCode, moduleToReadFrom, translatedErrorMessage);
        std::stringstream wss;
        wss << msg
            << std::endl
            << translatedErrorMessage
            << " (win32 code: "
            << std::to_string(errorCode)
            << ")";
        return wss.str();
    }

    std::string CreateErrorStringFromCode(const std::string msg, const DWORD errorCode) noexcept
    {
        std::string translatedErrorMessage;
        GetErrorCodeString(errorCode, translatedErrorMessage);
        std::stringstream wss;
        wss << msg
            << std::endl
            << translatedErrorMessage
            << " (win32 code: "
            << std::to_string(errorCode)
            << ")";
        return wss.str();
    }

    std::wstring GetErrorFromHResult(const std::wstring& msg, const HRESULT hr) noexcept
    {
        std::wstringstream ss;
        _com_error ce(hr);
        ss
            << msg
            << std::endl
            << ce.ErrorMessage()
            << L" (HRESULT: "
            << std::to_wstring(hr)
            << ")";
        return ss.str();
    }

    std::string GetErrorFromHResult(const std::string& msg, const HRESULT hr) noexcept
    {
        try
        {
            std::stringstream ss;
            _com_error ce(hr);
            ss
                << msg
                << std::endl
                << Strings::ConvertWStringToString(ce.ErrorMessage())
                << " (HRESULT: "
                << std::to_string(hr)
                << ")";
            return ss.str();
        }
        catch (const std::exception& ex)
        {
            std::wcerr << __FUNCSIG__ << ": " << ex.what() << std::endl;
            return "";
        }
    }
}