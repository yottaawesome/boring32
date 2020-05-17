#include "pch.hpp"
#include <winhttp.h>
#include "include/Error/Error.hpp"

namespace Boring32::Error
{
	std::wstring GetErrorCodeWString(const DWORD errorCode)
	{
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

        void* ptrMsgBuf;
        FormatMessage(
            flags,
            handle,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // TODO this is deprecated
            (LPTSTR)&ptrMsgBuf,
            0,
            nullptr);

        std::wstring msg((LPTSTR)ptrMsgBuf);
        LocalFree(ptrMsgBuf);

        return msg;
	}
}