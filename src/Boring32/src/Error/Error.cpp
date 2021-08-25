#include "pch.hpp"
#include <winhttp.h>
#include "include/Strings/Strings.hpp"
#include "include/Error/Error.hpp"
#include "Boring32/include/Library/Library.hpp"

// TODO: clean up
namespace Boring32::Error
{
    std::stringstream& PrintExceptionToStringStream(
        const std::exception& ex,
        std::stringstream& ss
    )
    {
        ss << ex.what() << std::endl;
        try
        {
            std::rethrow_if_nested(ex);
            return ss;
        }
        catch (const std::exception& ne)
        {
            return PrintExceptionToStringStream(ne, ss);
        }
    }

    std::wstringstream& PrintExceptionToStringStream(
        const std::exception& ex,
        std::wstringstream& ss
    )
    {
        ss << ex.what() << std::endl;
        try
        {
            std::rethrow_if_nested(ex);
            return ss;
        }
        catch (const std::exception& ne)
        {
            return PrintExceptionToStringStream(ne, ss);
        }
    }

    std::string GetNtStatusError(const std::string& msg, const NTSTATUS errorCode)
    {
        // See the caveats for NTSTATUS with FormatMessage here
        // https://stackoverflow.com/questions/7915215/how-do-i-convert-a-win32-exception-code-to-a-string
        std::string stringToHoldMessage = "";
        HMODULE handle = LoadLibraryW(L"ntdll.dll");
        if (handle == nullptr)
            return "Could not load ntdll.dll";

        GetErrorCodeString(errorCode, handle, stringToHoldMessage);
        if (handle)
            FreeLibrary(handle);
        return msg + ": " + stringToHoldMessage;
    }

    void GetErrorCodeString(const DWORD errorCode, std::string& stringToHoldMessage) noexcept
    {
        HMODULE moduleToLoad = nullptr;
        if (errorCode >= WINHTTP_ERROR_BASE && errorCode <= WINHTTP_ERROR_LAST)
            moduleToLoad = GetModuleHandleW(L"winhttp.dll");
        
        GetErrorCodeString(errorCode, moduleToLoad, stringToHoldMessage);
    }

    void GetErrorCodeString(const DWORD errorCode, HMODULE moduleToReadFrom, std::string& stringToHoldMessage) noexcept
    {
        stringToHoldMessage = "";
        DWORD flags =
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS;
        if (moduleToReadFrom)
            flags |= FORMAT_MESSAGE_FROM_HMODULE;

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
        stringToHoldMessage = L"";
        DWORD flags =
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS;
        if (moduleToReadFrom)
            flags |= FORMAT_MESSAGE_FROM_HMODULE;
        
        void* ptrMsgBuf = nullptr;
        FormatMessageW(
            flags,
            moduleToReadFrom,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // TODO this is deprecated
            (LPWSTR)&ptrMsgBuf,
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
        HMODULE handle = nullptr;
        if (errorCode >= WINHTTP_ERROR_BASE && errorCode <= WINHTTP_ERROR_LAST)
            handle = LoadLibrary(L"winhttp.dll");
        GetErrorCodeString(errorCode, handle, stringToHoldMessage);
        if (handle)
            FreeLibrary(handle);
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

    std::wstring CreateErrorStringFromNtStatus(
        const std::wstring msg,
        const DWORD errorCode
    ) noexcept
    {
        HMODULE handle = LoadLibraryW(L"ntdll.dll");
        if (handle == nullptr)
            return msg + L": could not load ntdll.dll";

        std::wstring translatedErrorMessage;
        GetErrorCodeString(errorCode, handle, translatedErrorMessage);
        std::wstringstream wss;
        wss << msg
            << std::endl
            << translatedErrorMessage
            << L" (ntstatus code: "
            << std::to_wstring(errorCode)
            << L")";
        FreeLibrary(handle);
        return wss.str();
    }

    std::string CreateErrorStringFromNtStatus(
        const std::string msg,
        const DWORD errorCode
    ) noexcept
    {
        HMODULE handle = LoadLibraryW(L"ntdll.dll");
        if (handle == nullptr)
            return msg + ": could not load ntdll.dll";

        std::string translatedErrorMessage;
        GetErrorCodeString(errorCode, handle, translatedErrorMessage);
        std::stringstream wss;
        wss << msg
            << std::endl
            << translatedErrorMessage
            << " (ntstatus code: "
            << std::to_string(errorCode)
            << ")";
        FreeLibrary(handle);
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
                << Strings::ToString(ce.ErrorMessage())
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

    void TranslateErrorCode(const DWORD errorCode, std::wstring& out) noexcept
    {
        TranslateErrorCode(nullptr, errorCode, out);
    }

    void TranslateErrorCode(const HMODULE moduleToReadFrom, const DWORD errorCode, std::wstring& out) noexcept
    {
        void* ptrMsgBuf = nullptr;
        DWORD flags =
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS;
        if (moduleToReadFrom)
            flags |= FORMAT_MESSAGE_FROM_HMODULE;

        // See https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-formatmessagew
        FormatMessageW(
            flags,              // dwFlags
            moduleToReadFrom,   // lpSource
            errorCode,          // dwMessageId
            LANG_USER_DEFAULT,  // dwLanguageId
            (LPWSTR)&ptrMsgBuf, // lpBuffer
            0,                  // nSize
            nullptr             // Arguments
        );
        if (ptrMsgBuf == nullptr)
        {
            out = L"Failed to translate Win32 error code: " + std::to_wstring(errorCode);
            return;
        }

        out = (LPWSTR)ptrMsgBuf;
        out += L" (error code: " + std::to_wstring(errorCode) + L")";
        LocalFree(ptrMsgBuf);
    }

    void TranslateErrorCode(const DWORD errorCode, std::string& out) noexcept
    {
        TranslateErrorCode(nullptr, errorCode, out);
    }

    void TranslateErrorCode(const HMODULE moduleToReadFrom, const DWORD errorCode, std::string& out) noexcept
    {
        void* ptrMsgBuf = nullptr;
        DWORD flags =
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS;
        if (moduleToReadFrom)
            flags |= FORMAT_MESSAGE_FROM_HMODULE;

        // See https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-formatmessagew
        FormatMessageA(
            flags,              // dwFlags
            moduleToReadFrom,   // lpSource
            errorCode,          // dwMessageId
            LANG_USER_DEFAULT,  // dwLanguageId
            (LPSTR)&ptrMsgBuf, // lpBuffer
            0,                  // nSize
            nullptr             // Arguments
        );
        if (ptrMsgBuf == nullptr)
        {
            out = "Failed to translate Win32 error code: " + std::to_string(errorCode);
            return;
        }

        out = (LPSTR)ptrMsgBuf;
        out += " (error code: " + std::to_string(errorCode) + ")";
        LocalFree(ptrMsgBuf);
    }
}