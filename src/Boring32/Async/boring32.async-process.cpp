module boring32.async:process;
import boring32.error;
import <stdexcept>;

namespace Boring32::Async
{
	Process::Process(
		std::wstring executablePath,
		std::wstring commandLine,
		std::wstring startingDirectory,
		const bool canInheritHandles
	)
	:	m_executablePath(std::move(executablePath)),
		m_commandLine(std::move(commandLine)),
		m_startingDirectory(std::move(startingDirectory)),
		m_canInheritHandles(canInheritHandles)
	{ }

	Process::Process(
		std::wstring executablePath,
		std::wstring commandLine,
		std::wstring startingDirectory,
		const bool canInheritHandles,
		const DWORD creationFlags,
		const STARTUPINFO& dataSi
	)
	:	m_executablePath(std::move(executablePath)),
		m_commandLine(std::move(commandLine)),
		m_startingDirectory(std::move(startingDirectory)),
		m_canInheritHandles(canInheritHandles),
		m_creationFlags(creationFlags),
		m_dataSi(dataSi)
	{ }

	void Process::Start()
	{
		if (m_executablePath.empty() && m_commandLine.empty())
			throw Error::Boring32Error("No executable path or command line set");

		PROCESS_INFORMATION processInfo{ 0 };
		m_dataSi.cb = sizeof(m_dataSi);
		// https://docs.microsoft.com/en-us/windows/win32/procthread/creating-processes
		const bool successfullyCreatedProcess =
			CreateProcessW(
				m_executablePath.empty()
					? nullptr : m_executablePath.c_str(),		// Module name
				m_commandLine.empty()
					? nullptr  : m_commandLine.data(), // Command line
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
		if (!successfullyCreatedProcess)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("Failed to create process", lastError);
		}

		m_process = processInfo.hProcess;
		m_thread = processInfo.hThread;
		m_processId = processInfo.dwProcessId;
		m_threadId = processInfo.dwThreadId;
	}

	void Process::CloseHandles()
	{
		CloseProcessHandle();
		CloseThreadHandle();
	}

	void Process::CloseProcessHandle()
	{
		m_process.Close();
	}

	void Process::CloseThreadHandle()
	{
		m_thread.Close();
	}

	HANDLE Process::GetProcessHandle() const noexcept
	{
		return m_process.GetHandle();
	}

	HANDLE Process::GetThreadHandle() const noexcept
	{
		return m_thread.GetHandle();
	}

	std::wstring Process::GetExecutablePath() const noexcept
	{
		return m_executablePath;
	}

	std::wstring Process::GetCommandLineStr() const noexcept
	{
		return m_commandLine;
	}

	std::wstring Process::GetStartingDirectory() const noexcept
	{
		return m_startingDirectory;
	}

	bool Process::GetHandlesInheritability() const noexcept
	{
		return m_canInheritHandles;
	}

	DWORD Process::GetCreationFlags() const noexcept
	{
		return m_creationFlags;
	}

	DWORD Process::GetProcessExitCode() const
	{
		if (!m_process) 
			throw Error::Boring32Error("No process handle to query");

		DWORD exitCode = 0;
		if (!GetExitCodeProcess(m_process.GetHandle(), &exitCode))
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("Failed to determine process exit code", lastError);
		}

		return exitCode;
	}
}
