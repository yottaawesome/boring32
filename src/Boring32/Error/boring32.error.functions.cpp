module;

#include <string>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <source_location>
#include <algorithm>
#include <format>
#include <Windows.h>

module boring32.error.functions;

namespace Boring32::Error
{
    void PrintExceptionInfo(const std::exception& e)
    {
        std::wcout << e.what() << std::endl;
        try
        {
            rethrow_if_nested(e);
        }
        catch (const std::exception& ne)
        {
            PrintExceptionInfo(ne);
        }
    }

    std::string FormatErrorMessage(
        const std::string& errorType,
        const std::source_location& location,
        const std::string& message
    )
    {
        return std::format(
            "{}: {} error at {}() in {}:{}:{}",
            message,
            errorType,
            location.function_name(),
            location.file_name(),
            location.line(),
            location.column()
        );
    }

    std::string FormatErrorMessage(
        const std::string& errorType,
        const std::source_location& location, 
        const std::string& message, 
        const DWORD errorCode,
        const std::string& translatedError
    )
    {
        return std::format(
            "{}: {} error {:#010X} ({}) at {}() in {}:{}:{}",
            message,
            errorType,
            errorCode,
            translatedError,
            location.function_name(),
            location.file_name(),
            location.line(),
            location.column()
        );
    }

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

    std::string FormatCode<std::string>(const DWORD errorCode, const DWORD flags, HMODULE moduleToSearch)
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
            std::wcerr << std::format(
                L"{}: LocalFree() failed: {}\n", 
                L"FormatCode()", 
                GetLastError()
            );
        
        std::erase_if(msg, [](const char x) { return x == '\n' || x == '\r'; });

        return msg;
    }

    std::wstring FormatCode<std::wstring>(const DWORD errorCode, const DWORD flags, HMODULE moduleToSearch)
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
        
        std::erase_if(msg, [](const wchar_t x) { return x == '\n' || x == '\r'; });

        return msg;
    }
}