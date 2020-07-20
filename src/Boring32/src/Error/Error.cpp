#include "pch.hpp"
#include <winhttp.h>
#include "include/Error/Error.hpp"

namespace Boring32::Error
{
    void GetErrorCodeString(const DWORD errorCode, std::string& stringToHoldMessage)
    {
        stringToHoldMessage = "";

        DWORD flags =
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS;
        HMODULE handle = nullptr;
        if (errorCode >= WINHTTP_ERROR_BASE && errorCode <= WINHTTP_ERROR_LAST)
        {
            GetModuleHandleEx(0, TEXT("winhttp.dll"), &handle);
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

        if(ptrMsgBuf != nullptr)
            stringToHoldMessage = (LPSTR)ptrMsgBuf;
        LocalFree(ptrMsgBuf);
    }

	void GetErrorCodeString(const DWORD errorCode, std::wstring& stringToHoldMessage)
	{
        stringToHoldMessage = L"";
        DWORD flags =
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS;
        HMODULE handle = nullptr;
        if (errorCode >= WINHTTP_ERROR_BASE && errorCode <= WINHTTP_ERROR_LAST)
        {
            GetModuleHandleEx(0, TEXT("winhttp.dll"), &handle);
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
            nullptr);

        if(ptrMsgBuf != nullptr)
            stringToHoldMessage = (LPTSTR)ptrMsgBuf;
        LocalFree(ptrMsgBuf);
	}

    std::wstring CreateErrorStringFromCode(const std::wstring msg, const DWORD errorCode)
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

    std::string CreateErrorStringFromCode(const std::string msg, const DWORD errorCode)
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
}