module;

#include <Windows.h>

module boring32.async.processinfo;
import boring32.raii.win32handle;

namespace Boring32::Async
{
	ProcessInfo::ProcessInfo()
	:	m_processInfo{ 0 }
	{ }

	ProcessInfo::~ProcessInfo()
	{
		if (m_processInfo.hProcess != nullptr)
		{
			CloseHandle(m_processInfo.hProcess);
			m_processInfo.hProcess = nullptr;
		}
		if (m_processInfo.hThread != nullptr)
		{
			CloseHandle(m_processInfo.hThread);
			m_processInfo.hThread = nullptr;
		}
	}

	ProcessInfo::ProcessInfo(const ProcessInfo& other)
		: m_processInfo{ 0 }
	{
		Copy(other);
	}

	void ProcessInfo::operator=(const ProcessInfo& other)
	{
		Copy(other);
	}

	void ProcessInfo::Copy(const ProcessInfo& other)
	{
		m_processInfo.dwProcessId = other.m_processInfo.dwProcessId;
		m_processInfo.dwThreadId = other.m_processInfo.dwThreadId;
		m_processInfo.hProcess = Raii::Win32Handle::DuplicatePassedHandle(other.m_processInfo.hProcess, false);
		m_processInfo.hThread = Raii::Win32Handle::DuplicatePassedHandle(other.m_processInfo.hThread, false);
	}

	ProcessInfo::ProcessInfo(ProcessInfo&& other) noexcept
		: m_processInfo{ 0 }
	{
		Move(other);
	}
	
	void ProcessInfo::operator=(ProcessInfo&& other) noexcept
	{
		Move(other);
	}

	void ProcessInfo::Move(ProcessInfo& other) noexcept
	{
		m_processInfo.dwProcessId = other.m_processInfo.dwProcessId;
		m_processInfo.dwThreadId = other.m_processInfo.dwThreadId;
		m_processInfo.hProcess = other.m_processInfo.hProcess;
		m_processInfo.hThread = other.m_processInfo.hThread;
		m_processInfo.hProcess = nullptr;
		m_processInfo.hThread = nullptr;
	}

	PROCESS_INFORMATION& ProcessInfo::GetProcessInfo() noexcept
	{
		return m_processInfo;
	}

	const PROCESS_INFORMATION& ProcessInfo::GetProcessInfo() const noexcept
	{
		return m_processInfo;
	}

	PROCESS_INFORMATION* ProcessInfo::operator&() noexcept
	{
		return &m_processInfo;
	}

	HANDLE ProcessInfo::GetProcessHandle() const noexcept
	{
		return m_processInfo.hProcess;
	}

	HANDLE ProcessInfo::GetThreadHandle() const noexcept
	{
		return m_processInfo.hThread;
	}
}