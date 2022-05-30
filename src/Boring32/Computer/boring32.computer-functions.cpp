module;

#include <string>
#include <source_location>
#include <Windows.h>

module boring32.computer:functions;
import boring32.error;

namespace Boring32::Computer
{
    std::wstring GetFormattedHostName(const COMPUTER_NAME_FORMAT format)
    {
        // TODO: find a way to support both ANSI and unicode versions
        DWORD bufferCharacterSize = 0; // this will include the trailing null byte
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getcomputernameexw
        if (!GetComputerNameExW(format, nullptr, &bufferCharacterSize))
        {
            const auto lastError = GetLastError();
            // ERROR_MORE_DATA initially is the expected error from MSDN link. 
            if (lastError != ERROR_MORE_DATA) throw Error::Win32Error(
                std::source_location::current(),
                "GetComputerNameExW() failed",
                lastError
            );
        }
        if (!bufferCharacterSize)
            return {};

        std::wstring returnVal(bufferCharacterSize, '\0');
        if (!GetComputerNameExW(format, &returnVal[0], &bufferCharacterSize))
        {
            const auto lastError = GetLastError();
            throw Error::Win32Error(
                std::source_location::current(),
                "GetComputerNameExW() failed",
                lastError
            );
        }
        // On output, receives the number of TCHARs copied to the destination buffer, 
        // not including the terminating null character.
        returnVal.resize(bufferCharacterSize);
        return returnVal;
    }
}