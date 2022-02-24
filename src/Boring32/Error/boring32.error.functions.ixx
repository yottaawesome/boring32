module;

#include <string>
#include <iostream>
#include <source_location>
#include <memory>
#include <Windows.h>

export module boring32.error.functions;

export namespace Boring32::Error
{
    [[noreturn]] void ThrowNested(const std::exception& ex1, const std::exception& ex2);

    std::string FormatErrorMessage(
        const std::string& errorType,
        const std::source_location& location,
        const std::string& message
    );

    std::string FormatErrorMessage(
        const std::string& errorType,
        const std::source_location& location, 
        const std::string& message,
        const DWORD errorCode,
        const std::string& translatedError
    );

    template<typename S>
    S FormatCode(const DWORD errorCode, const DWORD flags, HMODULE moduleToSearch) { return S(); }

    template<>
    std::string FormatCode<std::string>(const DWORD errorCode, const DWORD flags, HMODULE moduleToSearch);

    template<>
    std::wstring FormatCode<std::wstring>(const DWORD errorCode, const DWORD flags, HMODULE moduleToSearch);

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

    std::stringstream& PrintExceptionToStringStream(
        const std::exception& ex,
        std::stringstream& ss
    );

    std::wstringstream& PrintExceptionToStringStream(
        const std::exception& ex,
        std::wstringstream& ss
    );
}
