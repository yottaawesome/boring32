#include "pch.hpp"
#include <stdexcept>
#include "include/Async/Process.hpp"

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
		m_threadId(0)
	{ }

	Process::Process(
		const std::wstring& executablePath,
		const std::wstring& commandLine,
		const std::wstring& startingDirectory,
		const bool canInheritHandles,
		const DWORD creationFlags
	)
	:	m_executablePath(executablePath),
		m_commandLine(commandLine),
		m_startingDirectory(startingDirectory),
		m_canInheritHandles(canInheritHandles),
		m_creationFlags(creationFlags),
		m_processId(0),
		m_threadId(0),
		m_process(nullptr, false),
		m_thread(nullptr, false)
	{ }

	Process::Process(const Process& other)
	{
		Duplicate(other);
	}

	void Process::operator=(Process& other)
	{
		CloseHandles();
		Duplicate(other);
	}

	Process::Process(Process&& other) noexcept
	{
		Move(other);
	}

	void Process::operator=(Process&& other) noexcept
	{
		CloseHandles();
		Move(other);
	}

	void Process::Move(Process& other) noexcept
	{
		m_executablePath = std::move(other.m_executablePath);
		m_commandLine = std::move(other.m_commandLine);
		m_startingDirectory = std::move(other.m_startingDirectory);
		m_canInheritHandles = other.m_canInheritHandles;
		m_creationFlags = other.m_creationFlags;
		m_processId = other.m_processId;
		m_threadId = other.m_threadId;
		m_process = std::move(other.m_process);
		m_thread = std::move(other.m_thread);
	}

	void Process::Duplicate(const Process& other)
	{
		m_executablePath = other.m_executablePath;
		m_commandLine = other.m_commandLine;
		m_startingDirectory = other.m_startingDirectory;
		m_canInheritHandles = other.m_canInheritHandles;
		m_creationFlags = other.m_creationFlags;
		m_processId = other.m_processId;
		m_threadId = other.m_threadId;
		m_process = other.m_process;
		m_thread = other.m_thread;
	}

	void Process::Start()
	{
		if(m_executablePath == L"" && m_commandLine == L"")
			throw std::runtime_error("No executable path or command line set");

		STARTUPINFO dataSi{ 0 };
		PROCESS_INFORMATION processInfo;
		dataSi.cb = sizeof(dataSi);
		// https://docs.microsoft.com/en-us/windows/win32/procthread/creating-processes
		bool successfullyCreatedProcess =
			CreateProcess(
				m_executablePath != L"" 
					? m_executablePath.c_str()
					: nullptr,		// Module name
				m_commandLine != L"" 
					? m_commandLine.data()
					: nullptr,			// Command line
				nullptr,				// Process handle not inheritable
				nullptr,				// Thread handle not inheritable
				m_canInheritHandles,	// Set handle inheritance
				m_creationFlags,		// Creation flags
				nullptr,				// Use parent's environment block
				m_startingDirectory != L"" // Starting directory 
					? m_startingDirectory.c_str()
					: nullptr,				
				&dataSi,				// Pointer to STARTUPINFO structure
				&processInfo			// Pointer to PROCESS_INFORMATION structure
			);
		if (successfullyCreatedProcess == false)
			throw std::runtime_error("Failed to create process");

		m_process = Raii::Win32Handle(processInfo.hProcess, false);
		m_thread = Raii::Win32Handle(processInfo.hThread, false);
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

	HANDLE Process::GetProcessHandle()
	{
		return m_process.GetHandle();
	}

	HANDLE Process::GetThreadHandle()
	{
		return m_thread.GetHandle();
	}

	std::wstring Process::GetExecutablePath()
	{
		return m_executablePath;
	}

	std::wstring Process::GetCommandLineStr()
	{
		return m_commandLine;
	}

	std::wstring Process::GetStartingDirectory()
	{
		return m_startingDirectory;
	}

	bool Process::GetHandlesInheritability()
	{
		return m_canInheritHandles;
	}

	DWORD Process::GetCreationFlags()
	{
		return m_creationFlags;
	}
}
