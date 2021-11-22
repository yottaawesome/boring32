module;

#include <string>
#include <iostream>
#include <memory>
#include <Windows.h>

export module boring32.error.functions;

export namespace Boring32::Error
{
    template<typename T, typename S> 
        requires std::is_same<S,char>::value || std::is_same<S, wchar_t>::value
    bool TryCatchLogToWCerr(const T& function, const S* string) noexcept try
    {
        function();
        return true;
    }
    catch (const std::exception& ex)
    {
        std::wcerr
            << string
            << L" "
            << ex.what()
            << std::endl;
        return false;
    }

    template<typename T>
    bool TryCatchLogToWCerr(const T& function, const std::string& string) noexcept
    {
        return TryCatchLogToWCerr(function, string.c_str());
    }

    template<typename T>
    bool TryCatchLogToWCerr(const T& function, const std::wstring& string) noexcept
    {
        return TryCatchLogToWCerr(function, string.c_str());
    }

    template<typename S, typename...Args>
    bool TryCatchLogToWCerr(
        const auto function,
        const auto type,
        const S& string,
        Args&&...args
    ) noexcept
    {
        try
        {
            //((*type).*function)(std::forward<Args>(args)...);
            (type->*function)(std::forward<Args>(args)...);
            return true;
        }
        catch (const std::exception& ex)
        {
            std::wcerr
                << string
                << L" "
                << ex.what()
                << std::endl;
            return false;
        }
    }

    template<typename S>
    struct ErrorFormatter
    {
        static S FormatCode(const DWORD errorCode, const DWORD flags, HMODULE moduleToSearch) { static_assert(false); }
    };

    template<>
    struct ErrorFormatter<std::string>
    {
        static std::string FormatCode(const DWORD errorCode, const DWORD flags, HMODULE moduleToSearch);
    };

    template<>
    struct ErrorFormatter<std::wstring>
    {
        static std::wstring FormatCode(const DWORD errorCode, const DWORD flags, HMODULE moduleToSearch);
    };

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
        const STR_T errorString = ErrorFormatter<STR_T>::FormatCode(errorCode, flags, moduleHandle);
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

    /// <summary>
    /// 
    /// </summary>
    /// <typeparam name="STR_T"></typeparam>
    /// <param name="errorCode"></param>
    /// <returns></returns>
    template<typename STR_T>
    STR_T GetNtStatusCode(const DWORD errorCode)
    {
        /*struct Deleter
        {
            void operator()(void* library)
            {
                if (library) FreeLibrary(static_cast<HMODULE>(library));
            }
        };

        using LibPtr = std::unique_ptr<void, Deleter>;
        LibPtr lib(LoadLibraryW(L"ntdll.dll"));*/
        return TranslateErrorCode<STR_T>(errorCode, L"ntdll.dll");
    }

    std::stringstream& PrintExceptionToStringStream(
        const std::exception& ex,
        std::stringstream& ss
    );

    std::wstringstream& PrintExceptionToStringStream(
        const std::exception& ex,
        std::wstringstream& ss
    );
}

module :private;
namespace Boring32::Error
{
    /*struct Deleter 
    { 
        void operator()(void* library) 
        { 
            if (library) FreeLibrary(static_cast<HMODULE>(library)); 
        } 
    };*/
}
