export module boring32:async.processinfo;
import :win32;
import :raii;

export namespace Boring32::Async
{
	struct ProcessInfo final
	{
		~ProcessInfo()
		{
			Close();
		}

		ProcessInfo() = default;

		ProcessInfo(const ProcessInfo& other)
			: m_processInfo{ 0 }
		{
			Copy(other);
		}

		ProcessInfo(ProcessInfo&& other) noexcept
			: m_processInfo{ 0 }
		{
			Move(other);
		}

		ProcessInfo& operator=(const ProcessInfo& other)
		{
			return Copy(other);
		}

		ProcessInfo& operator=(ProcessInfo&& other) noexcept
		{
			return Move(other);
		}
		
		void Close()
		{
			if (m_processInfo.hProcess)
			{
				Win32::CloseHandle(m_processInfo.hProcess);
				m_processInfo.hProcess = nullptr;
			}
			if (m_processInfo.hThread)
			{
				Win32::CloseHandle(m_processInfo.hThread);
				m_processInfo.hThread = nullptr;
			}
		}

		Win32::PROCESS_INFORMATION& GetProcessInfo() noexcept
		{
			return m_processInfo;
		}

		const Win32::PROCESS_INFORMATION& GetProcessInfo() const noexcept
		{
			return m_processInfo;
		}

		Win32::PROCESS_INFORMATION* operator&() noexcept
		{
			return &m_processInfo;
		}

		Win32::HANDLE GetProcessHandle() const noexcept
		{
			return m_processInfo.hProcess;
		}

		 Win32::HANDLE GetThreadHandle() const noexcept
		{
			return m_processInfo.hThread;
		}

		private:
		ProcessInfo& Copy(const ProcessInfo& other)
		{
			if (this == &other)
				return *this;
			Close();
			m_processInfo.dwProcessId = other.m_processInfo.dwProcessId;
			m_processInfo.dwThreadId = other.m_processInfo.dwThreadId;
			m_processInfo.hProcess = RAII::Win32Handle::DuplicatePassedHandle(other.m_processInfo.hProcess, false);
			m_processInfo.hThread = RAII::Win32Handle::DuplicatePassedHandle(other.m_processInfo.hThread, false);
			return *this;
		}

		ProcessInfo& Move(ProcessInfo& other) noexcept
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

		Win32::PROCESS_INFORMATION m_processInfo{ 0 };
	};
}