#include "pch.hpp"
#include <stdexcept>
#include "include/Win32Utils.hpp"

namespace Win32Utils::IPC
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
		m_processInfo{ 0 },
		m_creationFlags(0)
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
		m_processInfo{ 0 },
		m_creationFlags(creationFlags)
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

	void Process::Move(Process& other)
	{
		m_executablePath = std::move(other.m_executablePath);
		m_commandLine = std::move(other.m_commandLine);
		m_startingDirectory = std::move(other.m_startingDirectory);
		m_canInheritHandles = other.m_canInheritHandles;
		m_processInfo = other.m_processInfo;
		m_creationFlags = other.m_creationFlags;
		other.m_processInfo.hProcess = nullptr;
		other.m_processInfo.hThread = nullptr;
	}

	void Process::Duplicate(const Process& other)
	{
		m_executablePath = other.m_executablePath;
		m_commandLine = other.m_commandLine;
		m_startingDirectory = other.m_startingDirectory;
		m_canInheritHandles = other.m_canInheritHandles;
		m_processInfo = other.m_processInfo;
		m_creationFlags = other.m_creationFlags;
		if (other.m_processInfo.hProcess)
		{
			m_processInfo.hProcess = nullptr;
			DuplicateHandle(
				GetCurrentProcess(),
				other.m_processInfo.hProcess,
				GetCurrentProcess(),
				&m_processInfo.hProcess,
				0,
				m_canInheritHandles,
				DUPLICATE_SAME_ACCESS
			);
			if (m_processInfo.hProcess == nullptr)
				throw std::runtime_error("Failed to duplicate process handle");
		}
		if (other.m_processInfo.hThread)
		{
			m_processInfo.hProcess = nullptr;
			DuplicateHandle(
				GetCurrentProcess(),
				other.m_processInfo.hThread,
				GetCurrentProcess(),
				&m_processInfo.hThread,
				0,
				m_canInheritHandles,
				DUPLICATE_SAME_ACCESS
			);
			if (m_processInfo.hThread == nullptr)
				throw std::runtime_error("Failed to duplicate thread handle");
		}
	}

	void Process::Start()
	{
		STARTUPINFO dataSi{ 0 };
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
				&m_processInfo			// Pointer to PROCESS_INFORMATION structure
			);
	}

	void Process::CloseHandles()
	{
		CloseProcessHandle();
		CloseThreadHandle();
	}

	void Process::CloseProcessHandle()
	{
		if (m_processInfo.hProcess)
		{
			CloseHandle(m_processInfo.hProcess);
			m_processInfo.hProcess = nullptr;
		}
	}

	void Process::CloseThreadHandle()
	{
		if (m_processInfo.hThread)
		{
			CloseHandle(m_processInfo.hThread);
			m_processInfo.hThread = nullptr;
		}
	}

	HANDLE Process::GetProcessHandle()
	{
		return m_processInfo.hProcess;
	}

	HANDLE Process::GetThreadHandle()
	{
		return m_processInfo.hThread;
	}
}
