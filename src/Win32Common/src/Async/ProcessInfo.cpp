#include "pch.hpp"
#include "include/Async.hpp"

namespace Win32Utils::Async
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