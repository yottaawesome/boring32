export module boring32.computer:processinfo;
import std;

import <win32.hpp>;
import boring32.raii;
import boring32.error;
import :functions;

export namespace Boring32::Computer
{
	struct ProcessTimes
	{
		size_t CreationTime = 0;
		size_t ExitTime = 0;
		size_t KernelTime = 0;
		size_t UserTime = 0;
		size_t ProcessTime = 0;
	};

	class ProcessInfo final
	{
		public:
			~ProcessInfo() = default;
			ProcessInfo() = default;
			ProcessInfo(const ProcessInfo&) = default;
			ProcessInfo(ProcessInfo&&) noexcept = default;
			ProcessInfo& operator=(ProcessInfo&) = default;
			ProcessInfo& operator=(ProcessInfo&&) noexcept = default;

		public:
			ProcessInfo(const HANDLE hProcess)
				: m_processHandle(hProcess)
			{
				if (!m_processHandle)
					throw Error::Boring32Error("hProcess cannot be null");
			}

			ProcessInfo(const DWORD processId)
			{
				if (!processId)
					throw Error::Boring32Error("processId must be a non-zero value");

				// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-openprocess
				m_processHandle = OpenProcess(PROCESS_QUERY_INFORMATION, false, processId);
				if (!m_processHandle)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("OpenProcess() failed", lastError);
				}
			}

		public:
			ProcessTimes GetTimes() const
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
					throw Error::Win32Error("GetProcessTimes() failed", lastError);
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

			std::wstring GetPath() const
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
					throw Error::Win32Error("GetModuleFileNameExW() failed", lastError);
				}
				return path.c_str();
			}

			DWORD GetID() const
			{
				if (!m_processHandle)
					throw Error::Boring32Error("m_processHandle cannot be null");
				// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocessid
				const DWORD id = GetProcessId(m_processHandle.GetHandle());
				if (!id)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("GetProcessId() failed", lastError);
				}
				return id;
			}

			DWORD GetHandleCount() const
			{
				if (!m_processHandle)
					throw Error::Boring32Error("m_processHandle cannot be null");

				DWORD handleCount;
				// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocesshandlecount
				const bool succeeded = GetProcessHandleCount(
					m_processHandle.GetHandle(),
					&handleCount
				);
				if (!succeeded)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("GetProcessHandleCount() failed", lastError);
				}
				return handleCount;
			}

			DWORD GetExitCode() const
			{
				if (!m_processHandle)
					throw Error::Boring32Error("m_processHandle cannot be null");

				DWORD exitCode;
				// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getexitcodeprocess
				const bool succeeded = GetExitCodeProcess(
					m_processHandle.GetHandle(),
					&exitCode
				);
				if (!succeeded)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("GetExitCodeProcess() failed", lastError);
				}
				return exitCode;
			}

		public:
			static std::vector<ProcessInfo> FromCurrentProcesses()
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

		private:
			RAII::Win32Handle m_processHandle;
	};
}