module;

#include <source_location>;

export module boring32.computer:functions;
import <string>;
import <vector>;
import <win32.hpp>;
import boring32.error;

export namespace Boring32::Computer
{
    [[nodiscard]]
    std::wstring GetFormattedHostName(const COMPUTER_NAME_FORMAT format)
    {
        // TODO: find a way to support both ANSI and unicode versions
        DWORD bufferCharacterSize = 0; // this will include the trailing null byte
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getcomputernameexw
        if (!GetComputerNameExW(format, nullptr, &bufferCharacterSize))
        {
            const auto lastError = GetLastError();
            // ERROR_MORE_DATA initially is the expected error from MSDN link. 
            if (lastError != ERROR_MORE_DATA)
                throw Error::Win32Error("GetComputerNameExW() failed", lastError);
        }
        if (!bufferCharacterSize)
            return {};

        std::wstring returnVal(bufferCharacterSize, '\0');
        if (!GetComputerNameExW(format, &returnVal[0], &bufferCharacterSize))
        {
            const auto lastError = GetLastError();
            throw Error::Win32Error("GetComputerNameExW() failed", lastError);
        }
        // On output, receives the number of TCHARs copied to the destination buffer, 
        // not including the terminating null character.
        returnVal.resize(bufferCharacterSize);
        return returnVal;
    }

    [[nodiscard]]
    size_t GetTotalMemoryKB()
    {
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getphysicallyinstalledsystemmemory
        size_t memoryInKB;
        if (!GetPhysicallyInstalledSystemMemory(&memoryInKB))
        {
            const auto lastError = GetLastError();
            throw Error::Win32Error(
                "GetPhysicallyInstalledSystemMemory() failed",
                lastError
            );
        }
        return memoryInKB;
    }

    [[nodiscard]]
    MEMORYSTATUSEX GetMemoryStatus()
    {
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-globalmemorystatusex
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/ns-sysinfoapi-memorystatusex
        MEMORYSTATUSEX memoryStatus{ .dwLength = sizeof(MEMORYSTATUSEX) };
        if (!GlobalMemoryStatusEx(&memoryStatus))
        {
            const auto lastError = GetLastError();
            throw Error::Win32Error("GlobalMemoryStatusEx() failed", lastError);
        }
        return memoryStatus;
    }

    [[nodiscard]]
    size_t GetMillisecondsSinceSystemBooted()
    {
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-gettickcount64
        return GetTickCount64();
    }

    struct TimeAdjustment
    {
        DWORD Adjustment = 0;
        DWORD Increment = 0;
        bool AdjustmentsAreEnabled = 0;
    };

    [[nodiscard]]
    TimeAdjustment GetSystemTimeAdjustmentInfo()
    {
        TimeAdjustment ts{};
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getsystemtimeadjustment
        const bool succeeded = GetSystemTimeAdjustment(
            &ts.Adjustment,
            &ts.Increment,
            reinterpret_cast<BOOL*>(&ts.AdjustmentsAreEnabled)
        );
        if (!succeeded)
        {
            const auto lastError = GetLastError();
            throw Error::Win32Error(
                "GetSystemTimeAdjustment() failed",
                lastError
            );
        }

        // invert the bool since the semantics are the opposite
        ts.AdjustmentsAreEnabled = !ts.AdjustmentsAreEnabled;

        return ts;
    }

    [[nodiscard]]
    SYSTEM_INFO GetSystemInfo() noexcept
    {
        // Can probably break this down to more useful pieces of info
        SYSTEM_INFO result;
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getsysteminfo
        // See also https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getnativesysteminfo
        GetSystemInfo(&result);
        return result;
    }

    // See https://docs.microsoft.com/en-us/windows/win32/api/winnt/ne-winnt-logical_processor_relationship
    [[nodiscard]] 
    std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX> GetLogicalProcessorInfo(
        const LOGICAL_PROCESSOR_RELATIONSHIP relationship
    )
    {
        DWORD lengthInBytes = 0;
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getlogicalprocessorinformationex
        GetLogicalProcessorInformationEx(
            relationship,
            nullptr,
            &lengthInBytes
        );
        DWORD lastError = GetLastError();
        if (lastError == ERROR_SUCCESS)
            return {};
        if (lastError != ERROR_INSUFFICIENT_BUFFER)
        {
            lastError = GetLastError();
            throw Error::Win32Error(
                "GetLogicalProcessorInformationEx() failed",
                lastError
            );
        }

        // https://docs.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-system_logical_processor_information_ex
        std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX> returnValue(
            lengthInBytes / sizeof(LOGICAL_PROCESSOR_RELATIONSHIP)
        );
        const bool succeeded = GetLogicalProcessorInformationEx(
            relationship,
            &returnValue[0],
            &lengthInBytes
        );
        if (!succeeded)
        {
            lastError = GetLastError();
            throw Error::Win32Error(
                "GetLogicalProcessorInformationEx() failed",
                lastError
            );
        }
        // In case it changes somehow
        returnValue.resize(lengthInBytes / sizeof(LOGICAL_PROCESSOR_RELATIONSHIP));

        return returnValue;
    }

    [[nodiscard]]
    std::vector<DWORD> EnumerateProcessIDs()
    {
        // Get the list of process identifiers.
        std::vector<DWORD> processes(1024);
        DWORD bytesNeeded;
        // https://docs.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-enumprocesses
        const bool succeeded = EnumProcesses(
            &processes[0],
            static_cast<DWORD>(processes.size()) * sizeof(DWORD),
            &bytesNeeded
        );
        if (!succeeded)
        {
            const auto lastError = GetLastError();
            throw Error::Win32Error("EnumProcesses() failed", lastError);
        }

        processes.resize(bytesNeeded / sizeof(DWORD));
        return processes;
    }

    [[nodiscard]]
    std::vector<void*> EnumerateDeviceDriverLoadAddresses()
    {
        // Get the list of process identifiers.
        std::vector<void*> deviceDriverAddresses(1024);
        DWORD bytesNeeded;
        // https://docs.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-enumdevicedrivers
        const bool succeeded = EnumDeviceDrivers(
            &deviceDriverAddresses[0],
            static_cast<DWORD>(deviceDriverAddresses.size()) * sizeof(void*),
            &bytesNeeded
        );
        if (!succeeded)
        {
            const auto lastError = GetLastError();
            throw Error::Win32Error("EnumDeviceDrivers() failed", lastError);
        }

        deviceDriverAddresses.resize(bytesNeeded / sizeof(void*));
        return deviceDriverAddresses;
    }
}
