export module boring32:error.functions;
import boring32.win32;
import :concepts;

namespace Boring32::Error
{
    auto FormatStackTrace(const std::stacktrace& trace) -> std::string
    {
        constexpr std::string_view fmt = 
R"(Entry:
    Description: {}
    Source file: {}
    Source line: {}
)";

        std::string bt;
        for (const std::stacktrace_entry& ste : trace)
        {
            // Break on this to avoid logging VC runtime functions
            if (ste.description().contains("invoke_main"))
                break;
            bt += std::format(fmt, ste.description(), ste.source_file(), ste.source_file());
        }
        return bt;
    }

    template<typename TString>
    struct ErrorCodeFormat final
    {
        static auto Format(
            Win32::DWORD errorCode,
            Win32::DWORD flags,
            Win32::HMODULE moduleToSearch
        ) -> TString
        {
            void* messageBuffer = nullptr;
            auto FormatStringFn =
                [] static constexpr
                {
                    if constexpr (std::same_as<std::string, TString>)
                        return Win32::FormatMessageA;
                    else if constexpr (std::same_as<std::wstring, TString>)
                        return Win32::FormatMessageW;
				}();
            FormatStringFn(
                flags,
                moduleToSearch,
                errorCode,
                0,
                reinterpret_cast<TString::pointer>(&messageBuffer),
                0,
                nullptr
            );
            if (not messageBuffer)
            {
                auto lastError = Win32::GetLastError();
                return
                    [errorCode, lastError]
                    {
                        if constexpr (std::same_as<std::string, TString>)
                            return std::format("FormatMessage() failed on code {} with error {}", errorCode, lastError);
                        else if constexpr (std::same_as<std::wstring, TString>)
                            return std::format(L"FormatMessage() failed on code {} with error {}", errorCode, lastError);
                    }();
            }

            TString msg(static_cast<TString::pointer>(messageBuffer));
            // This should never happen
            // See also https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-raisefailfastexception
            if (Win32::LocalFree(messageBuffer))
                Win32::__fastfail(Win32::FailFast::FatalExit);

            std::erase_if(msg, [](const TString::value_type x) { return x == '\n' || x == '\r'; });
            return msg;
        }
    };

    auto PrintExceptionToString(const std::exception& ex, std::string& ss) -> std::string&
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
}

namespace Boring32::Error::Formats
{
    constexpr std::string_view A = "{}: error at {}() in {}:{}:{}.\nStacktrace:\n{}";
    constexpr std::string_view B = "{}: {} error at {}() in {}:{}:{}.\nStacktrace:\n{}";
    constexpr std::string_view C = "{}: {} error {:#010X} ({}) at {}() in {}:{}:{}.\nStacktrace:\n{}";
}

export namespace Boring32::Error
{
    auto PrintExceptionToString(const std::exception& ex) -> std::string
    {
        std::string ss;
        return PrintExceptionToString(ex, ss);
    }

    // Parameters need to be templated, because throwing via
    // reference type will cause copy construction of the wrong
    // type, effectively causing slicing.
    // You *must* call GetLastError() before invoking this function
    // for system calls where GetLastError() may return useful
    // info.
    template <typename EX1, typename EX2>
    [[noreturn]] 
    void ThrowNested(EX1&& ex1, EX2&& ex2) try
    {
        throw ex1;
    }
    catch (...)
    {
        throw_with_nested(ex2);
    }

    auto FormatErrorMessage(
        const std::stacktrace& trace,
        const std::source_location& location,
        std::string_view message
    ) -> std::string
    {
        return std::format(
            Formats::A,
            message,
            location.function_name(),
            location.file_name(),
            location.line(),
            location.column(),
            FormatStackTrace(trace)
        );
    }

    auto FormatErrorMessage(
        std::string_view errorType,
        const std::stacktrace& trace,
        const std::source_location& location,
        std::string_view message
    ) -> std::string
    {
        return std::format(
            Formats::B,
            message,
            errorType,
            location.function_name(),
            location.file_name(),
            location.line(),
            location.column(),
            FormatStackTrace(trace)
        );
    }

    auto FormatErrorMessage(
        std::string_view errorType,
        const std::stacktrace& trace,
        const std::source_location& location,
        std::string_view message,
        const Win32::DWORD errorCode,
        std::string_view translatedError
    ) -> std::string
    {
        return std::format(
            Formats::C,
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

    // Translates Win32 errors, including COM errors, to human-readable error strings.
    // Some error codes are defined in specific modules; pass in the module as the 
    // second parameter for the function to translate such error codes.
    template<Concepts::WideOrNarrowString TString>
    auto TranslateErrorCode(
        const Win32::DWORD errorCode, 
        std::wstring_view moduleName = L""
    ) -> TString
    {
        // Retrieve the system error message for the last-error code
        Win32::HMODULE moduleHandle = moduleName.empty() ? nullptr : Win32::LoadLibraryW(moduleName.data());
        const Win32::DWORD flags =
            Win32::FormatMessageAllocateBuffer |
            Win32::FormatMessageFromSystem |
            Win32::FormatMessageIgnoreInserts |
            (moduleHandle ? Win32::FormatMessageFromHModule : 0);
        TString errorString = ErrorCodeFormat<TString>::Format(errorCode, flags, moduleHandle);
        if (moduleHandle)
            Win32::FreeLibrary(moduleHandle);

        return errorString;
    }

    template<Concepts::WideOrNarrowString TString>
    auto GetNtStatusCode(const Win32::DWORD errorCode) -> TString
    {
        return TranslateErrorCode<TString>(errorCode, L"ntdll.dll");
    }
}
