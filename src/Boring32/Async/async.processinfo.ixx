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

		auto operator=(const ProcessInfo& other) -> ProcessInfo&
		{
			return Copy(other);
		}

		auto operator=(ProcessInfo&& other) noexcept -> ProcessInfo&
		{
			return Move(other);
		}
		
		auto Close() -> void
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

		auto GetProcessInfo() noexcept -> Win32::PROCESS_INFORMATION&
		{
			return m_processInfo;
		}

		auto GetProcessInfo() const noexcept -> const Win32::PROCESS_INFORMATION&
		{
			return m_processInfo;
		}

		auto operator&() noexcept -> Win32::PROCESS_INFORMATION*
		{
			return &m_processInfo;
		}

		auto GetProcessHandle() const noexcept -> Win32::HANDLE
		{
			return m_processInfo.hProcess;
		}

		 auto GetThreadHandle() const noexcept -> Win32::HANDLE
		{
			return m_processInfo.hThread;
		}

		private:
		auto Copy(const ProcessInfo& other) -> ProcessInfo&
		{
			if (this == &other)
				return *this;
			Close();
			m_processInfo.dwProcessId = other.m_processInfo.dwProcessId;
			m_processInfo.dwThreadId = other.m_processInfo.dwThreadId;
			m_processInfo.hProcess = RAII::UniqueHandle::DuplicatePassedHandle(other.m_processInfo.hProcess, false);
			m_processInfo.hThread = RAII::UniqueHandle::DuplicatePassedHandle(other.m_processInfo.hThread, false);
			return *this;
		}

		auto Move(ProcessInfo& other) noexcept -> ProcessInfo&
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