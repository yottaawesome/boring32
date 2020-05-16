#include "pch.hpp"
#include "include/Async/Async.hpp"
#include "include/Util/Util.hpp"

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

	PROCESS_INFORMATION& ProcessInfo::GetProcessInfo()
	{
		return m_processInfo;
	}

	ProcessInfo::ProcessInfo(const ProcessInfo& other)
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
		m_processInfo.hProcess = Util::DuplicatePassedHandle(other.m_processInfo.hProcess, false);
		m_processInfo.hThread = Util::DuplicatePassedHandle(other.m_processInfo.hThread, false);
	}

	ProcessInfo::ProcessInfo(ProcessInfo&& other) noexcept
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

	PROCESS_INFORMATION* ProcessInfo::operator&()
	{
		return &m_processInfo;
	}

	HANDLE ProcessInfo::GetProcessHandle()
	{
		return m_processInfo.hProcess;
	}

	HANDLE ProcessInfo::GetThreadHandle()
	{
		return m_processInfo.hThread;
	}
}