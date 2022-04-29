module;

#include <Windows.h>

module boring32.async:processinfo;
import boring32.raii;

namespace Boring32::Async
{
	ProcessInfo::ProcessInfo()
	:	m_processInfo{ 0 }
	{ }

	ProcessInfo::~ProcessInfo()
	{
		Close();
	}

	ProcessInfo::ProcessInfo(const ProcessInfo& other)
		: m_processInfo{ 0 }
	{
		Copy(other);
	}

	ProcessInfo::ProcessInfo(ProcessInfo&& other) noexcept
		: m_processInfo{ 0 }
	{
		Move(other);
	}

	ProcessInfo& ProcessInfo::operator=(const ProcessInfo& other)
	{
		return Copy(other);
	}

	ProcessInfo& ProcessInfo::operator=(ProcessInfo&& other) noexcept
	{
		return Move(other);
	}

	ProcessInfo& ProcessInfo::Copy(const ProcessInfo& other)
	{
		if (this == &other)
			return *this;
		Close();
		m_processInfo.dwProcessId = other.m_processInfo.dwProcessId;
		m_processInfo.dwThreadId = other.m_processInfo.dwThreadId;
		m_processInfo.hProcess = Raii::Win32Handle::DuplicatePassedHandle(other.m_processInfo.hProcess, false);
		m_processInfo.hThread = Raii::Win32Handle::DuplicatePassedHandle(other.m_processInfo.hThread, false);
		return *this;
	}

	ProcessInfo& ProcessInfo::Move(ProcessInfo& other) noexcept
	{
		Close();
		m_processInfo.dwProcessId = other.m_processInfo.dwProcessId;
		m_processInfo.dwThreadId = other.m_processInfo.dwThreadId;
		m_processInfo.hProcess = other.m_processInfo.hProcess;
		m_processInfo.hThread = other.m_processInfo.hThread;
		other.m_processInfo.hProcess = nullptr;
		other.m_processInfo.hThread = nullptr;
		return *this;
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

	void ProcessInfo::Close()
	{
		if (m_processInfo.hProcess)
		{
			CloseHandle(m_processInfo.hProcess);
			m_processInfo.hProcess = nullptr;
		}
		if (m_processInfo.hThread)
		{
			CloseHandle(m_processInfo.hThread);
			m_processInfo.hThread = nullptr;
		}
	}
}