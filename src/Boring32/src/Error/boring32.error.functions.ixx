module;

#include <string>
#include <iostream>
#include <memory>
#include <Windows.h>

export module boring32.error.functions;

export namespace Boring32::Error
{
    template<typename T, typename S>
    bool TryCatchLogToWCerr(const T& function, const S* string) noexcept
    {
        try
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
    }

    template<typename T, typename S>
    bool TryCatchLogToWCerr(const T& function, const S& string) noexcept
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
    template<typename STR_T, typename STR_V = STR_T::value_type>
    STR_T TranslateErrorCode(const DWORD errorCode, const std::wstring& moduleName)
    {
        static_assert(
            std::is_same<std::basic_string<char>, STR_T>::value || std::is_same<std::basic_string<wchar_t>, STR_T>::value, 
            __FUNCTION__ "(): STR_T must be either a std::string or std::wstring");

        // Retrieve the system error message for the last-error code
        void* messageBuffer = nullptr;
        HMODULE moduleHandle = moduleName.empty() ? nullptr : LoadLibraryW(moduleName.c_str());
        const DWORD flags =
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS |
            (moduleHandle ? FORMAT_MESSAGE_FROM_HMODULE : 0);

        if (std::is_same<STR_V, char>::value)
        {
            FormatMessageA(
                flags,
                moduleHandle,
                errorCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // TODO this is deprecated
                static_cast<char*>(messageBuffer),
                0,
                nullptr
            );
        }
        else
        {
            FormatMessageW(
                flags,
                moduleHandle,
                errorCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // TODO this is deprecated
                static_cast<wchar_t*>(messageBuffer),
                0,
                nullptr
            );
        }

        if (moduleHandle)
            FreeLibrary(moduleHandle);
        if (messageBuffer == nullptr)
            return STR_T();

        STR_T msg(static_cast<STR_V*>(messageBuffer));
        LocalFree(messageBuffer);

        return msg;
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
