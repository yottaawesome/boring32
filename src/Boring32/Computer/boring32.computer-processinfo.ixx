export module boring32.computer:processinfo;
import <string>;
import <vector>;
import <memory>;
import boring32.win32;
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
			ProcessInfo(const Win32::HANDLE hProcess)
				: m_processHandle(hProcess)
			{
				if (!m_processHandle)
					throw Error::Boring32Error("hProcess cannot be null");
			}

			ProcessInfo(const Win32::DWORD processId)
			{
				if (!processId)
					throw Error::Boring32Error("processId must be a non-zero value");

				// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-openprocess
				m_processHandle = Win32::OpenProcess(Win32::ProcessQueryInformation, false, processId);
				if (!m_processHandle)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("OpenProcess() failed", lastError);
				}
			}

		public:
			ProcessTimes GetTimes() const
			{
				if (!m_processHandle)
					throw Error::Boring32Error("m_processHandle cannot be null");

				Win32::FILETIME ftCreationTime{ 0 };
				Win32::FILETIME ftExitTime{ 0 };
				Win32::FILETIME ftKernelTime{ 0 };
				Win32::FILETIME ftUserTime{ 0 };
				// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocesstimes
				const bool success = Win32::GetProcessTimes(
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
					Win32::ULARGE_INTEGER{ ftCreationTime.dwLowDateTime, ftCreationTime.dwHighDateTime }.QuadPart;
				const size_t exitTime =
					Win32::ULARGE_INTEGER{ ftExitTime.dwLowDateTime, ftExitTime.dwHighDateTime }.QuadPart;
				const size_t kernelTime =
					Win32::ULARGE_INTEGER{ ftKernelTime.dwLowDateTime, ftKernelTime.dwHighDateTime }.QuadPart;
				const size_t userTime =
					Win32::ULARGE_INTEGER{ ftUserTime.dwLowDateTime, ftUserTime.dwHighDateTime }.QuadPart;
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
				const Win32::DWORD charactersCopied = Win32::K32GetModuleFileNameExW(
					m_processHandle.GetHandle(),
					nullptr,
					&path[0],
					static_cast<Win32::DWORD>(path.size())
				);
				if (!charactersCopied)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("K32GetModuleFileNameExW() failed", lastError);
				}
				return path.c_str();
			}

			Win32::DWORD GetID() const
			{
				if (!m_processHandle)
					throw Error::Boring32Error("m_processHandle cannot be null");
				// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocessid
				const Win32::DWORD id = Win32::GetProcessId(m_processHandle.GetHandle());
				if (!id)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("GetProcessId() failed", lastError);
				}
				return id;
			}

			Win32::DWORD GetHandleCount() const
			{
				if (!m_processHandle)
					throw Error::Boring32Error("m_processHandle cannot be null");

				Win32::DWORD handleCount;
				// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocesshandlecount
				const bool succeeded = Win32::GetProcessHandleCount(
					m_processHandle.GetHandle(),
					&handleCount
				);
				if (!succeeded)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("GetProcessHandleCount() failed", lastError);
				}
				return handleCount;
			}

			Win32::DWORD GetExitCode() const
			{
				if (!m_processHandle)
					throw Error::Boring32Error("m_processHandle cannot be null");

				Win32::DWORD exitCode;
				// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getexitcodeprocess
				const bool succeeded = Win32::GetExitCodeProcess(
					m_processHandle.GetHandle(),
					&exitCode
				);
				if (!succeeded)
				{
					const auto lastError = Win32::GetLastError();
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
				for (const Win32::DWORD id : processIDs)
					// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-openprocess
					if (Win32::HANDLE hProcess = Win32::OpenProcess(Win32::ProcessQueryInformation, false, id))
						processes.emplace_back(hProcess);
				return processes;
			}

		private:
			RAII::Win32Handle m_processHandle;
	};
}