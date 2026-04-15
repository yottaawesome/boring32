export module boring32:computer.functions;
import std;
import :win32;
import :error;
import :raii;
import :strings;

export namespace Boring32::Computer
{
    [[nodiscard]]
    auto GetFormattedHostName(const Win32::COMPUTER_NAME_FORMAT format) -> std::wstring
    {
        // TODO: find a way to support both ANSI and unicode versions
        auto bufferCharacterSize = Win32::DWORD{0}; // this will include the trailing null byte
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getcomputernameexw
        if (not Win32::GetComputerNameExW(format, nullptr, &bufferCharacterSize))
        {
            // ERROR_MORE_DATA initially is the expected error from MSDN link. 
            if (auto lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::MoreData)
                throw Error::Win32Error{lastError, "GetComputerNameExW() failed"};
        }
        if (not bufferCharacterSize)
            return {};

        auto returnVal = std::wstring(bufferCharacterSize, '\0');
        if (not Win32::GetComputerNameExW(format, &returnVal[0], &bufferCharacterSize))
            throw Error::Win32Error{Win32::GetLastError(), "GetComputerNameExW() failed"};
        // On output, receives the number of TCHARs copied to the destination buffer, 
        // not including the terminating null character.
        returnVal.resize(bufferCharacterSize);
        return returnVal;
    }

    [[nodiscard]]
    auto GetTotalMemoryKB() -> size_t
    {
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getphysicallyinstalledsystemmemory
        auto memoryInKB = size_t{};
        if (not Win32::GetPhysicallyInstalledSystemMemory(&memoryInKB))
            throw Error::Win32Error{Win32::GetLastError(), "GetPhysicallyInstalledSystemMemory() failed"};
        return memoryInKB;
    }

    [[nodiscard]]
    auto GetMemoryStatus() -> Win32::MEMORYSTATUSEX
    {
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-globalmemorystatusex
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/ns-sysinfoapi-memorystatusex
        auto memoryStatus = Win32::MEMORYSTATUSEX{ .dwLength = sizeof(Win32::MEMORYSTATUSEX) };
        if (not Win32::GlobalMemoryStatusEx(&memoryStatus))
            throw Error::Win32Error{Win32::GetLastError(), "GlobalMemoryStatusEx() failed"};
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
        bool AdjustmentsAreEnabled = false;
    };

    [[nodiscard]]
    auto GetSystemTimeAdjustmentInfo() -> TimeAdjustment
    {
        auto ts = TimeAdjustment{};
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getsystemtimeadjustment
        auto adjustmentsDisabled = Win32::BOOL{};
        bool succeeded = Win32::GetSystemTimeAdjustment(
            &ts.Adjustment,
            &ts.Increment,
            &adjustmentsDisabled
        );
        if (not succeeded)
            throw Error::Win32Error{Win32::GetLastError(), "GetSystemTimeAdjustment() failed"};
        ts.AdjustmentsAreEnabled = !adjustmentsDisabled;
        return ts;
    }

    [[nodiscard]]
    auto GetSystemInfo() noexcept -> Win32::SYSTEM_INFO
    {
        // Can probably break this down to more useful pieces of info
        auto result = Win32::SYSTEM_INFO{};
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
        auto lengthInBytes = Win32::DWORD{0};
        // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getlogicalprocessorinformationex
        Win32::GetLogicalProcessorInformationEx(
            relationship,
            nullptr,
            &lengthInBytes
        );
        auto lastError = Win32::GetLastError();
        if (lastError == Win32::ErrorCodes::Success)
            return {};
        if (lastError != Win32::ErrorCodes::InsufficientBuffer)
            throw Error::Win32Error(lastError, "GetLogicalProcessorInformationEx() failed");

        // https://docs.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-system_logical_processor_information_ex
        auto returnValue = std::vector<Win32::SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(
            lengthInBytes / sizeof(Win32::LOGICAL_PROCESSOR_RELATIONSHIP)
        );
        auto succeeded = Win32::GetLogicalProcessorInformationEx(
            relationship,
            &returnValue[0],
            &lengthInBytes
        );
        if (not succeeded)
            throw Error::Win32Error{Win32::GetLastError(), "GetLogicalProcessorInformationEx() failed"};
        // In case it changes somehow
        returnValue.resize(lengthInBytes / sizeof(Win32::LOGICAL_PROCESSOR_RELATIONSHIP));

        return returnValue;
    }

    [[nodiscard]]
    auto EnumerateProcessIDs() -> std::vector<Win32::DWORD>
    {
        // Get the list of process identifiers.
        auto processes = std::vector<Win32::DWORD>(1024);
        auto bytesNeeded = Win32::DWORD{};
        // https://docs.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-enumprocesses
        auto succeeded = Win32::K32EnumProcesses(
            &processes[0],
            static_cast<Win32::DWORD>(processes.size()) * sizeof(Win32::DWORD),
            &bytesNeeded
        );
        if (not succeeded)
            throw Error::Win32Error{Win32::GetLastError(), "EnumProcesses() failed"};

        processes.resize(bytesNeeded / sizeof(Win32::DWORD));
        return processes;
    }

    [[nodiscard]]
    auto EnumerateDeviceDriverLoadAddresses() -> std::vector<void*>
    {
        // Get the list of process identifiers.
        auto deviceDriverAddresses = std::vector<void*>(1024);
        Win32::DWORD bytesNeeded;
        // https://docs.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-enumdevicedrivers
        auto succeeded = Win32::K32EnumDeviceDrivers(
            &deviceDriverAddresses[0],
            static_cast<Win32::DWORD>(deviceDriverAddresses.size()) * sizeof(void*),
            &bytesNeeded
        );
        if (not succeeded)
            throw Error::Win32Error{Win32::GetLastError(), "K32EnumDeviceDrivers() failed"};
        deviceDriverAddresses.resize(bytesNeeded / sizeof(void*));
        return deviceDriverAddresses;
    }

    ///	Find a process IDs by process name.
    [[nodiscard]]
    auto GetProcessIDsByName(const std::wstring& processName, int sessionIdToMatch) -> std::vector<Win32::DWORD>
    {
        if (processName.empty())
            throw Error::Boring32Error("ProcessName cannot be empty.");

        // https://docs.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-createtoolhelp32snapshot
        auto processesSnapshot = RAII::Win32Handle{ Win32::CreateToolhelp32Snapshot(Win32::Th32csSnapProcess, 0) };
        if (processesSnapshot == Win32::InvalidHandleValue)
            throw Error::Win32Error{ Win32::GetLastError(), "CreateToolhelp32Snapshot() failed" };

        auto procEntry = Win32::PROCESSENTRY32W{ .dwSize = sizeof(Win32::PROCESSENTRY32W) };
        // https://docs.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-process32firstw
        if (not Win32::Process32FirstW(processesSnapshot.GetHandle(), &procEntry))
            throw Error::Win32Error{ Win32::GetLastError(), "Process32First() failed" };

        auto results = std::vector<Win32::DWORD>{};
        do
        {
            if (not Strings::DoCaseInsensitiveMatch(procEntry.szExeFile, processName))
                continue;
            if (sessionIdToMatch < 0)
            {
                results.push_back(procEntry.th32ProcessID);
                continue;
            }

            auto processSessionId = Win32::DWORD{};
            // https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-processidtosessionid
            if (not Win32::ProcessIdToSessionId(procEntry.th32ProcessID, &processSessionId))
                throw Error::Win32Error{ Win32::GetLastError(), "ProcessIdToSessionId() failed" };
            if (processSessionId == sessionIdToMatch)
                results.push_back(procEntry.th32ProcessID);
        } while (Win32::Process32NextW(processesSnapshot.GetHandle(), &procEntry));
        // https://docs.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-process32nextw

        return results;
    }
}
