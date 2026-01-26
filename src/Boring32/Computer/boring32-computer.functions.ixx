export module boring32:computer.functions;
import std;
import :win32;
import :error;

export namespace Boring32::Computer
{
    [[nodiscard]]
    auto GetFormattedHostName(const Win32::COMPUTER_NAME_FORMAT format) -> std::wstring
    {
        // TODO: find a way to support both ANSI and unicode versions
        Win32::DWORD bufferCharacterSize = 0; // this will include the trailing null byte
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getcomputernameexw
        if (not Win32::GetComputerNameExW(format, nullptr, &bufferCharacterSize))
        {
            // ERROR_MORE_DATA initially is the expected error from MSDN link. 
            if (auto lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::MoreData)
                throw Error::Win32Error(lastError, "GetComputerNameExW() failed");
        }
        if (not bufferCharacterSize)
            return {};

        std::wstring returnVal(bufferCharacterSize, '\0');
        if (not Win32::GetComputerNameExW(format, &returnVal[0], &bufferCharacterSize))
            throw Error::Win32Error(Win32::GetLastError(), "GetComputerNameExW() failed");
        // On output, receives the number of TCHARs copied to the destination buffer, 
        // not including the terminating null character.
        returnVal.resize(bufferCharacterSize);
        return returnVal;
    }

    [[nodiscard]]
    auto GetTotalMemoryKB() -> size_t
    {
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getphysicallyinstalledsystemmemory
        size_t memoryInKB;
        if (not Win32::GetPhysicallyInstalledSystemMemory(&memoryInKB))
            throw Error::Win32Error(Win32::GetLastError(), "GetPhysicallyInstalledSystemMemory() failed");
        return memoryInKB;
    }

    [[nodiscard]]
    auto GetMemoryStatus() -> Win32::MEMORYSTATUSEX
    {
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-globalmemorystatusex
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/ns-sysinfoapi-memorystatusex
        Win32::MEMORYSTATUSEX memoryStatus{ .dwLength = sizeof(Win32::MEMORYSTATUSEX) };
        if (not Win32::GlobalMemoryStatusEx(&memoryStatus))
            throw Error::Win32Error(Win32::GetLastError(), "GlobalMemoryStatusEx() failed");
        return memoryStatus;
    }

    [[nodiscard]]
    auto GetMillisecondsSinceSystemBooted() -> size_t
    {
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-gettickcount64
        return Win32::GetTickCount64();
    }

    struct TimeAdjustment
    {
        Win32::DWORD Adjustment = 0;
        Win32::DWORD Increment = 0;
        bool AdjustmentsAreEnabled = 0;
    };

    [[nodiscard]]
    auto GetSystemTimeAdjustmentInfo() -> TimeAdjustment
    {
        TimeAdjustment ts{};
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getsystemtimeadjustment
        bool succeeded = Win32::GetSystemTimeAdjustment(
            &ts.Adjustment,
            &ts.Increment,
            reinterpret_cast<Win32::BOOL*>(&ts.AdjustmentsAreEnabled)
        );
        if (not succeeded)
        {
            throw Error::Win32Error(Win32::GetLastError(), "GetSystemTimeAdjustment() failed");
        }

        // invert the bool since the semantics are the opposite
        ts.AdjustmentsAreEnabled = !ts.AdjustmentsAreEnabled;

        return ts;
    }

    [[nodiscard]]
    auto GetSystemInfo() noexcept -> Win32::SYSTEM_INFO
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
    auto GetLogicalProcessorInfo(Win32::LOGICAL_PROCESSOR_RELATIONSHIP relationship)
        -> std::vector<Win32::SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>
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
            throw Error::Win32Error(lastError, "GetLogicalProcessorInformationEx() failed");

        // https://docs.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-system_logical_processor_information_ex
        std::vector<Win32::SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX> returnValue(
            lengthInBytes / sizeof(Win32::LOGICAL_PROCESSOR_RELATIONSHIP)
        );
        bool succeeded = Win32::GetLogicalProcessorInformationEx(
            relationship,
            &returnValue[0],
            &lengthInBytes
        );
        if (not succeeded)
            throw Error::Win32Error(Win32::GetLastError(), "GetLogicalProcessorInformationEx() failed");
        // In case it changes somehow
        returnValue.resize(lengthInBytes / sizeof(Win32::LOGICAL_PROCESSOR_RELATIONSHIP));

        return returnValue;
    }

    [[nodiscard]]
    auto EnumerateProcessIDs() -> std::vector<Win32::DWORD>
    {
        // Get the list of process identifiers.
        std::vector<Win32::DWORD> processes(1024);
        Win32::DWORD bytesNeeded;
        // https://docs.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-enumprocesses
        bool succeeded = Win32::K32EnumProcesses(
            &processes[0],
            static_cast<Win32::DWORD>(processes.size()) * sizeof(Win32::DWORD),
            &bytesNeeded
        );
        if (not succeeded)
            throw Error::Win32Error(Win32::GetLastError(), "EnumProcesses() failed");

        processes.resize(bytesNeeded / sizeof(Win32::DWORD));
        return processes;
    }

    [[nodiscard]]
    auto EnumerateDeviceDriverLoadAddresses() -> std::vector<void*>
    {
        // Get the list of process identifiers.
        std::vector<void*> deviceDriverAddresses(1024);
        Win32::DWORD bytesNeeded;
        // https://docs.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-enumdevicedrivers
        bool succeeded = Win32::K32EnumDeviceDrivers(
            &deviceDriverAddresses[0],
            static_cast<Win32::DWORD>(deviceDriverAddresses.size()) * sizeof(void*),
            &bytesNeeded
        );
        if (not succeeded)
            throw Error::Win32Error(Win32::GetLastError(), "K32EnumDeviceDrivers() failed");

        deviceDriverAddresses.resize(bytesNeeded / sizeof(void*));
        return deviceDriverAddresses;
    }
}
