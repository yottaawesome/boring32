export module boring32:async.process;
import std;
import boring32.win32;
import :raii;
import :error;

export namespace Boring32::Async
{
	struct Process final
	{
		Process() = default;

		Process(Process&& other) noexcept = default;

		Process(const Process& other) = default;

		Process(
			std::wstring executablePath,
			std::wstring commandLine,
			std::wstring startingDirectory,
			const bool canInheritHandles
		) : m_executablePath(std::move(executablePath)),
			m_commandLine(std::move(commandLine)),
			m_startingDirectory(std::move(startingDirectory)),
			m_canInheritHandles(canInheritHandles)
		{ }

		Process(
			std::wstring executablePath,
			std::wstring commandLine,
			std::wstring startingDirectory,
			const bool canInheritHandles,
			const Win32::DWORD creationFlags,
			const Win32::STARTUPINFO& dataSi
		) : m_executablePath(std::move(executablePath)),
			m_commandLine(std::move(commandLine)),
			m_startingDirectory(std::move(startingDirectory)),
			m_canInheritHandles(canInheritHandles),
			m_creationFlags(creationFlags),
			m_dataSi(dataSi)
		{ }
			
		Process& operator=(Process&& other) noexcept = default;
		Process& operator=(const Process& other) = default;
		
		void Start()
		{
			if (m_executablePath.empty() && m_commandLine.empty())
				throw Error::Boring32Error("No executable path or command line set");

			Win32::PROCESS_INFORMATION processInfo{ 0 };
			m_dataSi.cb = sizeof(m_dataSi);
			// https://docs.microsoft.com/en-us/windows/win32/procthread/creating-processes
			const bool successfullyCreatedProcess =
				Win32::CreateProcessW(
					m_executablePath.empty()
						? nullptr : m_executablePath.c_str(),		// Module name
					m_commandLine.empty()
						? nullptr : m_commandLine.data(), // Command line
					nullptr,				// Process handle not inheritable
					nullptr,				// Thread handle not inheritable
					m_canInheritHandles,	// Set handle inheritance
					m_creationFlags,		// Creation flags
					nullptr,				// Use parent's environment block
					m_startingDirectory.empty() // Starting directory 
						? nullptr : m_startingDirectory.c_str(),
					&m_dataSi,				// Pointer to STARTUPINFO structure
					&processInfo			// Pointer to PROCESS_INFORMATION structure
				);
			if (not successfullyCreatedProcess)
				throw Error::Win32Error(Win32::GetLastError(), "Failed to create process");

			m_process = processInfo.hProcess;
			m_thread = processInfo.hThread;
			m_processId = processInfo.dwProcessId;
			m_threadId = processInfo.dwThreadId;
		}

		void CloseHandles()
		{
			CloseProcessHandle();
			CloseThreadHandle();
		}

		void CloseProcessHandle()
		{
			m_process.Close();
		}

		void CloseThreadHandle()
		{
			m_thread.Close();
		}

		Win32::HANDLE GetProcessHandle() const noexcept
		{
			return m_process.GetHandle();
		}

		Win32::HANDLE GetThreadHandle() const noexcept
		{
			return m_thread.GetHandle();
		}
			
		const std::wstring& GetExecutablePath() const noexcept
		{
			return m_executablePath;
		}

		const std::wstring& GetCommandLineStr() const noexcept
		{
			return m_commandLine;
		}

		const std::wstring& GetStartingDirectory() const noexcept
		{
			return m_startingDirectory;
		}

		bool GetHandlesInheritability() const noexcept
		{
			return m_canInheritHandles;
		}

		Win32::DWORD GetCreationFlags() const noexcept
		{
			return m_creationFlags;
		}

		Win32::DWORD GetProcessExitCode() const
		{
			if (not m_process)
				throw Error::Boring32Error("No process handle to query");

			Win32::DWORD exitCode = 0;
			if (not Win32::GetExitCodeProcess(m_process.GetHandle(), &exitCode))
				throw Error::Win32Error(Win32::GetLastError(), "Failed to determine process exit code");

			return exitCode;
		}

		private:
		std::wstring m_executablePath;
		std::wstring m_commandLine;
		std::wstring m_startingDirectory;
		bool m_canInheritHandles = false;
		Win32::DWORD m_creationFlags = 0;
		Boring32::RAII::Win32Handle m_process;
		Boring32::RAII::Win32Handle m_thread;
		Win32::DWORD m_processId = 0;
		Win32::DWORD m_threadId = 0;
		Win32::STARTUPINFO m_dataSi = { 0 };
	};
}