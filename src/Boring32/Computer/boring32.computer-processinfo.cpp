module;

#include <source_location>
#include <string>
#include <memory>
#include <vector>
#include <Windows.h>
#include <psapi.h>

module boring32.computer:processinfo;
import boring32.raii;
import boring32.error;
import :functions;

namespace Boring32::Computer
{
	ProcessInfo::~ProcessInfo() {}

	ProcessInfo::ProcessInfo(const ProcessInfo&) = default;
	ProcessInfo::ProcessInfo(ProcessInfo&&) noexcept = default;
	ProcessInfo& ProcessInfo::operator=(ProcessInfo&) = default;
	ProcessInfo& ProcessInfo::operator=(ProcessInfo&&) noexcept = default;

	ProcessInfo::ProcessInfo(HANDLE hProcess)
		: m_processHandle(hProcess)
	{
		if (!m_processHandle)
			throw Error::Boring32Error("hProcess cannot be null");
	}

	ProcessTimes ProcessInfo::GetTimes() const
	{
		if (!m_processHandle)
			throw Error::Boring32Error("m_processHandle cannot be null");

		FILETIME ftCreationTime{ 0 };
		FILETIME ftExitTime{ 0 };
		FILETIME ftKernelTime{ 0 };
		FILETIME ftUserTime{ 0 };
		// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocesstimes
		const bool success = GetProcessTimes(
			m_processHandle.GetHandle(),
			&ftCreationTime,
			&ftExitTime,
			&ftKernelTime,
			&ftUserTime
		);
		if (!success)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("GetProcessTimes() failed: {}", lastError);
		}

		const size_t startTime =
			ULARGE_INTEGER{ ftCreationTime.dwLowDateTime, ftCreationTime.dwHighDateTime }.QuadPart;
		const size_t exitTime =
			ULARGE_INTEGER{ ftExitTime.dwLowDateTime, ftExitTime.dwHighDateTime }.QuadPart;
		const size_t kernelTime =
			ULARGE_INTEGER{ ftKernelTime.dwLowDateTime, ftKernelTime.dwHighDateTime }.QuadPart;
		const size_t userTime =
			ULARGE_INTEGER{ ftUserTime.dwLowDateTime, ftUserTime.dwHighDateTime }.QuadPart;
		return {
			.CreationTime = startTime,
			.ExitTime = exitTime,
			.KernelTime = kernelTime,
			.UserTime = userTime,
			.ProcessTime = kernelTime + userTime
		};
	}

	std::wstring ProcessInfo::GetPath() const
	{
		if (!m_processHandle)
			throw Error::Boring32Error("m_processHandle cannot be null");

		std::wstring path(2048, '\0');
		// https://docs.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-getmodulefilenameexw
		// See also https://docs.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-getprocessimagefilenamew
		// See also https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-queryfullprocessimagenamew
		const DWORD charactersCopied = GetModuleFileNameExW(
			m_processHandle.GetHandle(),
			nullptr,
			&path[0],
			static_cast<DWORD>(path.size())
		);
		if (!charactersCopied)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("GetModuleFileNameExW() failed: {}", lastError);
		}
		return path.c_str();
	}

	DWORD ProcessInfo::GetID() const
	{
		if (!m_processHandle)
			throw Error::Boring32Error("m_processHandle cannot be null");
		const DWORD id = GetProcessId(m_processHandle.GetHandle());
		if (!id)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("GetProcessId() failed: {}", lastError);
		}
		return id;
	}

	std::vector<ProcessInfo> ProcessInfo::FromCurrentProcesses()
	{
		const auto processIDs = EnumerateProcessIDs();
		std::vector<ProcessInfo> processes;
		// Processes may not necessarily be openable due to permissions or
		// they may have exited between getting the IDs and opening a handle
		// to them, so all we can do is a best effort attempt to open them.
		for (const DWORD id : processIDs)
			// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-openprocess
			if (HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, false, id))
				processes.emplace_back(hProcess);
		return processes;
	}
}
