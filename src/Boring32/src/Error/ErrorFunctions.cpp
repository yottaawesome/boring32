module;

#include <string>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <format>
#include <Windows.h>

module boring32.error.functions;

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

    std::string ErrorFormatter<std::string>::FormatCode(const DWORD errorCode, const DWORD flags, HMODULE moduleToSearch)
    { 
        void* messageBuffer = nullptr;
        FormatMessageA(
            flags,
            moduleToSearch,
            errorCode,
            0,
            reinterpret_cast<char*>(&messageBuffer),
            0,
            nullptr
        );
        if (!messageBuffer)
            return std::format(
                "{}: FormatMessageA() failed on code {} with error {}", 
                __FUNCSIG__, 
                errorCode, 
                GetLastError()
            );

        std::string msg(static_cast<char*>(messageBuffer));
        if (LocalFree(messageBuffer))
            std::wcerr << std::format(L"{}: LocalFree() failed: {}\n", TEXT(__FUNCSIG__), GetLastError());

        return msg;
    }

    std::wstring ErrorFormatter<std::wstring>::FormatCode(const DWORD errorCode, const DWORD flags, HMODULE moduleToSearch)
    {
        void* messageBuffer = nullptr;
        FormatMessageW(
            flags,
            moduleToSearch,
            errorCode,
            0,
            reinterpret_cast<wchar_t*>(&messageBuffer),
            0,
            nullptr
        );
        if (!messageBuffer)
            return std::format(
                L"{}: FormatMessageA() failed on code {} with error {}", 
                TEXT(__FUNCSIG__), 
                errorCode, 
                GetLastError()
            );
        
        std::wstring msg(static_cast<wchar_t*>(messageBuffer));
        if (LocalFree(messageBuffer))
            std::wcerr << std::format(L"{}: LocalFree() failed: {}\n", TEXT(__FUNCSIG__), GetLastError());
        
        return msg;
    }
}