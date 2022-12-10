module;

#include <source_location>

module boring32.async:process;
import boring32.error;
import <stdexcept>;

namespace Boring32::Async
{
	Process::~Process()
	{
		CloseHandles();
	}

	Process::Process()
	:	m_executablePath(L""),
		m_commandLine(L""),
		m_startingDirectory(L""),
		m_canInheritHandles(false),
		m_creationFlags(0),
		m_processId(0),
		m_threadId(0),
		m_dataSi{0}
	{ }

	Process::Process(
		std::wstring executablePath,
		std::wstring commandLine,
		std::wstring startingDirectory,
		const bool canInheritHandles
	)
	:	m_executablePath(std::move(executablePath)),
		m_commandLine(std::move(commandLine)),
		m_startingDirectory(std::move(startingDirectory)),
		m_canInheritHandles(canInheritHandles),
		m_creationFlags(0),
		m_processId(0),
		m_threadId(0),
		m_process(nullptr),
		m_thread(nullptr),
		m_dataSi({0})
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
		m_processId(0),
		m_threadId(0),
		m_process(nullptr),
		m_thread(nullptr),
		m_dataSi(dataSi)
	{ }

	Process::Process(const Process& other)
	:	m_executablePath(L""),
		m_commandLine(L""),
		m_startingDirectory(L""),
		m_canInheritHandles(false),
		m_creationFlags(0),
		m_processId(0),
		m_threadId(0),
		m_dataSi{ 0 }
	{
		Copy(other);
	}

	Process& Process::operator=(Process& other)
	{
		return Copy(other);
	}

	Process::Process(Process&& other) noexcept
	:	m_executablePath(L""),
		m_commandLine(L""),
		m_startingDirectory(L""),
		m_canInheritHandles(false),
		m_creationFlags(0),
		m_processId(0),
		m_threadId(0),
		m_dataSi{ 0 }
	{
		Move(other);
	}

	Process& Process::operator=(Process&& other) noexcept
	{
		return Move(other);
	}

	Process& Process::Move(Process& other) noexcept
	{
		if (this == &other)
			return *this;

		CloseHandles();
		m_executablePath = std::move(other.m_executablePath);
		m_commandLine = std::move(other.m_commandLine);
		m_startingDirectory = std::move(other.m_startingDirectory);
		m_canInheritHandles = other.m_canInheritHandles;
		m_creationFlags = other.m_creationFlags;
		m_processId = other.m_processId;
		m_threadId = other.m_threadId;
		m_process = std::move(other.m_process);
		m_thread = std::move(other.m_thread);
		return *this;
	}

	Process& Process::Copy(const Process& other)
	{
		if (this == &other)
			return *this;

		CloseHandles();
		m_executablePath = other.m_executablePath;
		m_commandLine = other.m_commandLine;
		m_startingDirectory = other.m_startingDirectory;
		m_canInheritHandles = other.m_canInheritHandles;
		m_creationFlags = other.m_creationFlags;
		m_processId = other.m_processId;
		m_threadId = other.m_threadId;
		m_process = other.m_process;
		m_thread = other.m_thread;
		return *this;
	}

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
			throw Error::Win32Error("Failed to create process", GetLastError());

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
		m_process = nullptr;
	}

	void Process::CloseThreadHandle()
	{
		m_thread.Close();
		m_thread = nullptr;
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
