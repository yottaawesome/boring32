export module boring32:computer_processinfo;
import boring32.shared;
import :raii;
import :error;
import :computer_functions;

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

	struct ProcessInfo final
	{
		ProcessInfo() = default;
		ProcessInfo(const ProcessInfo&) = default;
		ProcessInfo(ProcessInfo&&) noexcept = default;
		ProcessInfo& operator=(ProcessInfo&) = default;
		ProcessInfo& operator=(ProcessInfo&&) noexcept = default;

		ProcessInfo(const Win32::HANDLE hProcess)
			: m_processHandle(hProcess)
		{
			if (not m_processHandle)
				throw Error::Boring32Error("hProcess cannot be null");
		}

		ProcessInfo(const Win32::DWORD processId)
		{
			if (not processId)
				throw Error::Boring32Error("processId must be a non-zero value");

			// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-openprocess
			m_processHandle = Win32::OpenProcess(Win32::ProcessQueryInformation, false, processId);
			if (auto lastError = Win32::GetLastError(); not m_processHandle)
				throw Error::Win32Error("OpenProcess() failed", lastError);
		}

		ProcessTimes GetTimes() const
		{
			if (not m_processHandle)
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
			if (auto lastError = GetLastError(); not success)
				throw Error::Win32Error("GetProcessTimes() failed", lastError);

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
			if (not m_processHandle)
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
			if (auto lastError = GetLastError(); not charactersCopied)
				throw Error::Win32Error("K32GetModuleFileNameExW() failed", lastError);
			return path.c_str();
		}

		Win32::DWORD GetID() const
		{
			if (not m_processHandle)
				throw Error::Boring32Error("m_processHandle cannot be null");
			// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocessid
			const Win32::DWORD id = Win32::GetProcessId(m_processHandle.GetHandle());
			if (auto lastError = Win32::GetLastError(); not id)
				throw Error::Win32Error("GetProcessId() failed", lastError);
			return id;
		}

		Win32::DWORD GetHandleCount() const
		{
			if (not m_processHandle)
				throw Error::Boring32Error("m_processHandle cannot be null");

			Win32::DWORD handleCount;
			// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocesshandlecount
			const bool succeeded = Win32::GetProcessHandleCount(
				m_processHandle.GetHandle(),
				&handleCount
			);
			if (auto lastError = Win32::GetLastError(); not succeeded)
				throw Error::Win32Error("GetProcessHandleCount() failed", lastError);
			return handleCount;
		}

		Win32::DWORD GetExitCode() const
		{
			if (not m_processHandle)
				throw Error::Boring32Error("m_processHandle cannot be null");

			Win32::DWORD exitCode;
			// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getexitcodeprocess
			const bool succeeded = Win32::GetExitCodeProcess(m_processHandle.GetHandle(), &exitCode);
			if (auto lastError = Win32::GetLastError(); not succeeded)
				throw Error::Win32Error("GetExitCodeProcess() failed", lastError);
			return exitCode;
		}

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