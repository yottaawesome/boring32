export module boring32:computer.processinfo;
import std;
import :win32;
import :raii;
import :error;
import :computer.functions;

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
		ProcessInfo() = default;

		ProcessInfo(Win32::HANDLE hProcess)
			: m_processHandle{ hProcess }
		{
			if (not m_processHandle)
				throw Error::Boring32Error{ "hProcess cannot be null" };
		}

		ProcessInfo(Win32::DWORD processId)
		{
			if (not processId)
				throw Error::Boring32Error{ "processId must be a non-zero value" };

			// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-openprocess
			m_processHandle = Win32::OpenProcess(Win32::ProcessQueryInformation, false, processId);
			if (not m_processHandle)
				throw Error::Win32Error{Win32::GetLastError(), "OpenProcess() failed"};
		}

		auto GetTimes() const -> ProcessTimes
		{
			if (not m_processHandle)
				throw Error::Boring32Error{ "m_processHandle cannot be null" };

			auto ftCreationTime = Win32::FILETIME{};
			auto ftExitTime = Win32::FILETIME{};
			auto ftKernelTime = Win32::FILETIME{};
			auto ftUserTime = Win32::FILETIME{};
			// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocesstimes
			auto success = Win32::GetProcessTimes(
				m_processHandle.GetHandle(),
				&ftCreationTime,
				&ftExitTime,
				&ftKernelTime,
				&ftUserTime
			);
			if (not success)
				throw Error::Win32Error{Win32::GetLastError(), "GetProcessTimes() failed"};

			auto startTime = std::uint64_t{
				Win32::ULARGE_INTEGER{ ftCreationTime.dwLowDateTime, ftCreationTime.dwHighDateTime }.QuadPart };
			auto exitTime = std::uint64_t{
				Win32::ULARGE_INTEGER{ ftExitTime.dwLowDateTime, ftExitTime.dwHighDateTime }.QuadPart };
			auto kernelTime = std::uint64_t{
				Win32::ULARGE_INTEGER{ ftKernelTime.dwLowDateTime, ftKernelTime.dwHighDateTime }.QuadPart };
			auto userTime = std::uint64_t{
				Win32::ULARGE_INTEGER{ ftUserTime.dwLowDateTime, ftUserTime.dwHighDateTime }.QuadPart };
			return {
				.CreationTime = startTime,
				.ExitTime = exitTime,
				.KernelTime = kernelTime,
				.UserTime = userTime,
				.ProcessTime = kernelTime + userTime
			};
		}

		auto GetPath() const -> std::wstring
		{
			if (not m_processHandle)
				throw Error::Boring32Error{ "m_processHandle cannot be null" };

			auto path = std::wstring(2048, '\0');
			// https://docs.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-getmodulefilenameexw
			// See also https://docs.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-getprocessimagefilenamew
			// See also https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-queryfullprocessimagenamew
			auto charactersCopied = 
				Win32::K32GetModuleFileNameExW(
					m_processHandle.GetHandle(),
					nullptr,
					&path[0],
					static_cast<Win32::DWORD>(path.size())
				);
			if (not charactersCopied)
				throw Error::Win32Error{Win32::GetLastError(), "K32GetModuleFileNameExW() failed"};
			return path.c_str();
		}

		auto GetID() const -> Win32::DWORD
		{
			if (not m_processHandle)
				throw Error::Boring32Error{ "m_processHandle cannot be null" };
			// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocessid
			auto id = Win32::DWORD{ Win32::GetProcessId(m_processHandle.GetHandle()) };
			if (not id)
				throw Error::Win32Error{Win32::GetLastError(), "GetProcessId() failed"};
			return id;
		}

		auto GetHandleCount() const -> Win32::DWORD
		{
			if (not m_processHandle)
				throw Error::Boring32Error{ "m_processHandle cannot be null" };

			auto handleCount = Win32::DWORD{};
			// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocesshandlecount
			auto succeeded = Win32::GetProcessHandleCount(m_processHandle.GetHandle(), &handleCount);
			if (not succeeded)
				throw Error::Win32Error{Win32::GetLastError(), "GetProcessHandleCount() failed"};
			return handleCount;
		}

		auto GetExitCode() const -> Win32::DWORD
		{
			if (not m_processHandle)
				throw Error::Boring32Error{ "m_processHandle cannot be null" };

			auto exitCode = Win32::DWORD{};
			// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getexitcodeprocess
			auto succeeded = Win32::GetExitCodeProcess(m_processHandle.GetHandle(), &exitCode);
			if (not succeeded)
				throw Error::Win32Error{Win32::GetLastError(), "GetExitCodeProcess() failed"};
			return exitCode;
		}

		static auto FromCurrentProcesses() -> std::vector<ProcessInfo>
		{
			auto processIDs = EnumerateProcessIDs();
			auto processes = std::vector<ProcessInfo>{};
			// Processes may not necessarily be openable due to permissions or
			// they may have exited between getting the IDs and opening a handle
			// to them, so all we can do is a best effort attempt to open them.
			for (Win32::DWORD id : processIDs)
				// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-openprocess
				if (auto hProcess = Win32::HANDLE{ Win32::OpenProcess(Win32::ProcessQueryInformation, false, id) })
					processes.emplace_back(hProcess);
			return processes;
		}

	private:
		RAII::UniqueHandle m_processHandle;
	};
}