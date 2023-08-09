export module boring32.async:process;
import std;
import <win32.hpp>;
import boring32.raii;
import boring32.error;

export namespace Boring32::Async
{
	class Process
	{
		public:
			virtual ~Process() = default;

			Process() = default;

			Process(Process&& other) noexcept = default;

			Process(const Process& other) = default;

			Process(
				std::wstring executablePath,
				std::wstring commandLine,
				std::wstring startingDirectory,
				const bool canInheritHandles
			) : m_executablePath(std::move(executablePath)),
				m_commandLine(std::move(commandLine)),
				m_startingDirectory(std::move(startingDirectory)),
				m_canInheritHandles(canInheritHandles)
			{ }

			Process(
				std::wstring executablePath,
				std::wstring commandLine,
				std::wstring startingDirectory,
				const bool canInheritHandles,
				const DWORD creationFlags,
				const STARTUPINFO& dataSi
			) : m_executablePath(std::move(executablePath)),
				m_commandLine(std::move(commandLine)),
				m_startingDirectory(std::move(startingDirectory)),
				m_canInheritHandles(canInheritHandles),
				m_creationFlags(creationFlags),
				m_dataSi(dataSi)
			{ }
			
		public:
			virtual Process& operator=(Process&& other) noexcept = default;
			virtual Process& operator=(const Process& other) = default;
		
		public:
			virtual void Start()
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
						? nullptr : m_commandLine.data(), // Command line
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

			virtual void CloseHandles()
			{
				CloseProcessHandle();
				CloseThreadHandle();
			}

			virtual void CloseProcessHandle()
			{
				m_process.Close();
			}

			virtual void CloseThreadHandle()
			{
				m_thread.Close();
			}

			virtual HANDLE GetProcessHandle() const noexcept
			{
				return m_process.GetHandle();
			}

			virtual HANDLE GetThreadHandle() const noexcept
			{
				return m_thread.GetHandle();
			}
			
			virtual const std::wstring& GetExecutablePath() const noexcept
			{
				return m_executablePath;
			}

			virtual const std::wstring& GetCommandLineStr() const noexcept
			{
				return m_commandLine;
			}

			virtual const std::wstring& GetStartingDirectory() const noexcept
			{
				return m_startingDirectory;
			}

			virtual bool GetHandlesInheritability() const noexcept
			{
				return m_canInheritHandles;
			}

			virtual DWORD GetCreationFlags() const noexcept
			{
				return m_creationFlags;
			}

			virtual DWORD GetProcessExitCode() const
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

		protected:
			std::wstring m_executablePath;
			std::wstring m_commandLine;
			std::wstring m_startingDirectory;
			bool m_canInheritHandles = false;
			DWORD m_creationFlags = 0;
			Boring32::RAII::Win32Handle m_process;
			Boring32::RAII::Win32Handle m_thread;
			DWORD m_processId = 0;
			DWORD m_threadId = 0;
			STARTUPINFO m_dataSi = { 0 };
	};
}