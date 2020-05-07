#include "pch.hpp"
#include <winhttp.h>
#include "include/Win32Utils.hpp"

namespace Win32Utils::Error
{
	std::wstring GetErrorCodeWString(const DWORD errorCode)
	{
        LPVOID lpMsgBuf;

        HMODULE handle = nullptr;
        DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS;
        if (errorCode >= WINHTTP_ERROR_BASE && errorCode <= WINHTTP_ERROR_LAST)
        {
            GetModuleHandleEx(0, TEXT("winhttp.dll"), &handle);
            flags |= FORMAT_MESSAGE_FROM_HMODULE;
        }

        FormatMessage(
            flags,
            handle,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // TODO this is deprecated
            (LPTSTR)&lpMsgBuf,
            0,
            nullptr);

        std::wstring msg((LPTSTR)lpMsgBuf);
        LocalFree(lpMsgBuf);

        return msg;
	}
}