export module boring32:computer_functions;
import boring32.shared;
import boring32.error;

export namespace Boring32::Computer
{
    [[nodiscard]]
    std::wstring GetFormattedHostName(const Win32::COMPUTER_NAME_FORMAT format)
    {
        // TODO: find a way to support both ANSI and unicode versions
        Win32::DWORD bufferCharacterSize = 0; // this will include the trailing null byte
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getcomputernameexw
        if (!Win32::GetComputerNameExW(format, nullptr, &bufferCharacterSize))
        {
            const auto lastError = Win32::GetLastError();
            // ERROR_MORE_DATA initially is the expected error from MSDN link. 
            if (lastError != Win32::ErrorCodes::MoreData)
                throw Error::Win32Error("GetComputerNameExW() failed", lastError);
        }
        if (!bufferCharacterSize)
            return {};

        std::wstring returnVal(bufferCharacterSize, '\0');
        if (!Win32::GetComputerNameExW(format, &returnVal[0], &bufferCharacterSize))
        {
            const auto lastError = Win32::GetLastError();
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
        if (!Win32::GetPhysicallyInstalledSystemMemory(&memoryInKB))
        {
            const auto lastError = Win32::GetLastError();
            throw Error::Win32Error(
                "GetPhysicallyInstalledSystemMemory() failed",
                lastError
            );
        }
        return memoryInKB;
    }

    [[nodiscard]]
    Win32::MEMORYSTATUSEX GetMemoryStatus()
    {
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-globalmemorystatusex
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/ns-sysinfoapi-memorystatusex
        Win32::MEMORYSTATUSEX memoryStatus{ .dwLength = sizeof(Win32::MEMORYSTATUSEX) };
        if (!Win32::GlobalMemoryStatusEx(&memoryStatus))
        {
            const auto lastError = Win32::GetLastError();
            throw Error::Win32Error("GlobalMemoryStatusEx() failed", lastError);
        }
        return memoryStatus;
    }

    [[nodiscard]]
    size_t GetMillisecondsSinceSystemBooted()
    {
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-gettickcount64
        return Win32::GetTickCount64();
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
        const bool succeeded = Win32::GetSystemTimeAdjustment(
            &ts.Adjustment,
            &ts.Increment,
            reinterpret_cast<Win32::BOOL*>(&ts.AdjustmentsAreEnabled)
        );
        if (!succeeded)
        {
            const auto lastError = Win32::GetLastError();
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
    Win32::SYSTEM_INFO GetSystemInfo() noexcept
    {
        // Can probably break this down to more useful pieces of info
        Win32::SYSTEM_INFO result;
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getsysteminfo
        // See also https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getnativesysteminfo
        Win32::GetSystemInfo(&result);
        return result;
    }

    // See https://docs.microsoft.com/en-us/windows/win32/api/winnt/ne-winnt-logical_processor_relationship
    [[nodiscard]] 
    std::vector<Win32::SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX> GetLogicalProcessorInfo(
        const Win32::LOGICAL_PROCESSOR_RELATIONSHIP relationship
    )
    {
        Win32::DWORD lengthInBytes = 0;
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getlogicalprocessorinformationex
        Win32::GetLogicalProcessorInformationEx(
            relationship,
            nullptr,
            &lengthInBytes
        );
        Win32::DWORD lastError = Win32::GetLastError();
        if (lastError == Win32::ErrorCodes::Success)
            return {};
        if (lastError != Win32::ErrorCodes::InsufficientBuffer)
        {
            lastError = Win32::GetLastError();
            throw Error::Win32Error(
                "GetLogicalProcessorInformationEx() failed",
                lastError
            );
        }

        // https://docs.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-system_logical_processor_information_ex
        std::vector<Win32::SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX> returnValue(
            lengthInBytes / sizeof(Win32::LOGICAL_PROCESSOR_RELATIONSHIP)
        );
        const bool succeeded = Win32::GetLogicalProcessorInformationEx(
            relationship,
            &returnValue[0],
            &lengthInBytes
        );
        if (!succeeded)
        {
            lastError = Win32::GetLastError();
            throw Error::Win32Error(
                "GetLogicalProcessorInformationEx() failed",
                lastError
            );
        }
        // In case it changes somehow
        returnValue.resize(lengthInBytes / sizeof(Win32::LOGICAL_PROCESSOR_RELATIONSHIP));

        return returnValue;
    }

    [[nodiscard]]
    std::vector<DWORD> EnumerateProcessIDs()
    {
        // Get the list of process identifiers.
        std::vector<Win32::DWORD> processes(1024);
        Win32::DWORD bytesNeeded;
        // https://docs.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-enumprocesses
        const bool succeeded = Win32::K32EnumProcesses(
            &processes[0],
            static_cast<Win32::DWORD>(processes.size()) * sizeof(Win32::DWORD),
            &bytesNeeded
        );
        if (!succeeded)
        {
            const auto lastError = Win32::GetLastError();
            throw Error::Win32Error("EnumProcesses() failed", lastError);
        }

        processes.resize(bytesNeeded / sizeof(Win32::DWORD));
        return processes;
    }

    [[nodiscard]]
    std::vector<void*> EnumerateDeviceDriverLoadAddresses()
    {
        // Get the list of process identifiers.
        std::vector<void*> deviceDriverAddresses(1024);
        Win32::DWORD bytesNeeded;
        // https://docs.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-enumdevicedrivers
        const bool succeeded = Win32::K32EnumDeviceDrivers(
            &deviceDriverAddresses[0],
            static_cast<Win32::DWORD>(deviceDriverAddresses.size()) * sizeof(void*),
            &bytesNeeded
        );
        if (!succeeded)
        {
            const auto lastError = Win32::GetLastError();
            throw Error::Win32Error("K32EnumDeviceDrivers() failed", lastError);
        }

        deviceDriverAddresses.resize(bytesNeeded / sizeof(void*));
        return deviceDriverAddresses;
    }
}
