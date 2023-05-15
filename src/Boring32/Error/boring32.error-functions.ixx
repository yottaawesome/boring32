export module boring32.error:functions;
import <string>;
import <iostream>;
import <memory>;
import <stdexcept>;
import <sstream>;
import <algorithm>;
import <format>;
import <stacktrace>;
import <source_location>;
import <win32.hpp>;

export namespace Boring32::Error
{
    // Parameters need to be templated, because throwing via
    // reference type will cause copy construction of the wrong
    // type, effectively causing slicing.
    // You *must* call GetLastError() before invoking this function
    // for system calls where GetLastError() may return useful
    // info.
    template <typename EX1, typename EX2>
    [[noreturn]] void ThrowNested(EX1&& ex1, EX2&& ex2) try
    {
        throw ex1;
    }
    catch (...)
    {
        throw_with_nested(ex2);
    }

    template <typename EX1, typename EX2>
    void ThrowNested2(const EX1* ex1, const EX2* ex2) try
    {
        throw ex1;
    }
    catch (...)
    {
        throw_with_nested(ex2);
    }

    void PrintExceptionInfo(const std::exception& e, const unsigned level = 0)
    {
        std::wcout << std::format("{}-> {}", std::string(level, '-'), e.what()).c_str() << std::endl;
        try
        {
            rethrow_if_nested(e);
        }
        catch (const std::exception& ne)
        {
            PrintExceptionInfo(ne, level + 1);
        }
    }

    std::string FormatStackTrace(const std::stacktrace& trace)
    {
        std::string bt;
        for (const std::stacktrace_entry& ste : trace)
        {
            // Break on this to avoid logging VC runtime functions
            if (ste.description().contains("invoke_main"))
                break;
            bt += std::format(
                "Entry:\n\tDescription: {}\n\tSource file: {}\n\tSource line: {}\n",
                ste.description(),
                ste.source_file(),
                ste.source_line()
            );
        }
        return bt;
    }

    std::string FormatErrorMessage(
        const std::stacktrace& trace,
        const std::source_location& location,
        const std::string& message
    )
    {
        return std::format(
            "{}: error at {}() in {}:{}:{}. Stacktrace:\n{}",
            message,
            location.function_name(),
            location.file_name(),
            location.line(),
            location.column(),
            FormatStackTrace(trace)
        );
    }

    std::string FormatErrorMessage(
        const std::string& errorType,
        const std::stacktrace& trace,
        const std::source_location& location,
        const std::string& message
    )
    {
        return std::format(
            "{}: {} error at {}() in {}:{}:{}. Stacktrace:\n{}",
            message,
            errorType,
            location.function_name(),
            location.file_name(),
            location.line(),
            location.column(),
            FormatStackTrace(trace)
        );
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
        const std::stacktrace& trace,
        const std::source_location& location,
        const std::string& message,
        const DWORD errorCode,
        const std::string& translatedError
    )
    {
        return std::format(
            "{}: {} error {:#010X} ({}) at {}() in {}:{}:{}. Stacktrace:\n{}",
            message,
            errorType,
            errorCode,
            translatedError,
            location.function_name(),
            location.file_name(),
            location.line(),
            location.column(),
            FormatStackTrace(trace)
        );
    }

    template<typename S>
    S FormatCode(
        const DWORD errorCode, 
        const DWORD flags, 
        HMODULE moduleToSearch
    ) { return S(); }

    template<>
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
        {
            const auto lastError = GetLastError();
            return std::format(
                "FormatMessageA() failed on code {} with error {}",
                errorCode,
                lastError
            );
        }

        std::string msg(static_cast<char*>(messageBuffer));
        if (LocalFree(messageBuffer))
        {
            const auto lastError = GetLastError();
            std::wcerr << std::format(
                L"LocalFree() failed: {}\n",
                lastError
            );
        }

        std::erase_if(msg, [](const char x) { return x == '\n' || x == '\r'; });

        return msg;
    }

    template<>
    std::wstring FormatCode<std::wstring>(
        const DWORD errorCode, 
        const DWORD flags, 
        HMODULE moduleToSearch
    )
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
        {
            const auto lastError = GetLastError();
            return std::format(
                L"FormatMessageA() failed on code {} with error {}",
                errorCode,
                lastError
            );
        }

        std::wstring msg(static_cast<wchar_t*>(messageBuffer));
        if (LocalFree(messageBuffer))
        {
            const auto lastError = GetLastError();
            std::wcerr << std::format(L"LocalFree() failed: {}\n", lastError);
        }

        std::erase_if(msg, [](const wchar_t x) { return x == '\n' || x == '\r'; });

        return msg;
    }

    /// <summary>
    ///     Translates Win32 errors, including COM errors, to human-readable error strings.
    ///     Some error codes are defined in specific modules; pass in the module as the 
    ///     second parameter for the function to translate such error codes.
    /// </summary>
    /// <typeparam name="STR_T">Either a wstring or string.</typeparam>
    /// <typeparam name="STR_V">Leave this to resolve to wchar_t or char.</typeparam>
    /// <param name="errorCode">The error code to translate.</param>
    /// <param name="moduleName">Optional. The module name to translate from.</param>
    /// <returns>The translated error string or a default error string if the function fails.</returns>
    template<typename STR_T>
    STR_T TranslateErrorCode(const DWORD errorCode, const std::wstring& moduleName)
        requires std::is_same<std::string, STR_T>::value || std::is_same<std::wstring, STR_T>::value
    {
        // Retrieve the system error message for the last-error code
        HMODULE moduleHandle = moduleName.empty() ? nullptr : LoadLibraryW(moduleName.c_str());
        const DWORD flags =
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS |
            (moduleHandle ? FORMAT_MESSAGE_FROM_HMODULE : 0);
        const STR_T errorString = FormatCode<STR_T>(errorCode, flags, moduleHandle);
        if (moduleHandle)
            FreeLibrary(moduleHandle);

        return errorString;
    }

    /// <summary>
    ///     Translates a default Win32 or COM error code.
    /// </summary>
    /// <typeparam name="STR_T"></typeparam>
    /// <param name="errorCode">The error code to translate.</param>
    /// <returns>The translated error string or a default error string if the function fails.</returns>
    template<typename STR_T>
    STR_T TranslateErrorCode(const DWORD errorCode)
    {
        return TranslateErrorCode<STR_T>(errorCode, L"");
    }

    template<typename STR_T>
    STR_T GetNtStatusCode(const DWORD errorCode)
    {
        return TranslateErrorCode<STR_T>(errorCode, L"ntdll.dll");
    }

    std::string& PrintExceptionToString(
        const std::exception& ex,
        std::string& ss
    )
    {
        ss += std::format("{}\n", ex.what());
        try
        {
            std::rethrow_if_nested(ex);
            return ss;
        }
        catch (const std::exception& ne)
        {
            return PrintExceptionToString(ne, ss);
        }
    }

    std::string PrintExceptionToString(
        const std::exception& ex
    )
    {
        std::string ss;
        return PrintExceptionToString(ex, ss);
    }
}
