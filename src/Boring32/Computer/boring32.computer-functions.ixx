module;

#include <string>
#include <vector>
#include <Windows.h>

export module boring32.computer:functions;

export namespace Boring32::Computer
{
    [[nodiscard]]
    std::wstring GetFormattedHostName(const COMPUTER_NAME_FORMAT format);

    [[nodiscard]]
    size_t GetTotalMemoryKB();

    [[nodiscard]]
    MEMORYSTATUSEX GetMemoryStatus();

    [[nodiscard]]
    size_t GetMillisecondsSinceSystemBooted();

    struct TimeAdjustment
    {
        DWORD Adjustment = 0;
        DWORD Increment = 0;
        bool AdjustmentsAreEnabled = 0;
    };

    [[nodiscard]]
    TimeAdjustment GetSystemTimeAdjustmentInfo();

    [[nodiscard]]
    SYSTEM_INFO GetSystemInfo();

    // See https://docs.microsoft.com/en-us/windows/win32/api/winnt/ne-winnt-logical_processor_relationship
    [[nodiscard]] 
    std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX> GetLogicalProcessorInfo(
        const LOGICAL_PROCESSOR_RELATIONSHIP relationship
    );

    [[nodiscard]]
    std::vector<DWORD> EnumerateProcessIDs();

    [[nodiscard]]
    std::vector<void*> EnumerateDeviceDriverLoadAddresses();
}
