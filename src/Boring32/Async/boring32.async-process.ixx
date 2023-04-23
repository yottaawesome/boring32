export module boring32.async:process;
import boring32.raii;
import <string>;
import <win32.hpp>;

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
			);
			Process(
				std::wstring executablePath,
				std::wstring commandLine,
				std::wstring startingDirectory,
				const bool canInheritHandles,
				const DWORD creationFlags,
				const STARTUPINFO& dataSi
			);
			
		public:
			virtual Process& operator=(Process&& other) noexcept = default;
			virtual Process& operator=(const Process& other) = default;
		
		public:
			virtual void Start();
			virtual void CloseHandles();
			virtual void CloseProcessHandle();
			virtual void CloseThreadHandle();
			virtual HANDLE GetProcessHandle() const noexcept;
			virtual HANDLE GetThreadHandle() const noexcept;
			virtual const std::wstring& GetExecutablePath() const noexcept;
			virtual const std::wstring& GetCommandLineStr() const noexcept;
			virtual const std::wstring& GetStartingDirectory() const noexcept;
			virtual bool GetHandlesInheritability() const noexcept;
			virtual DWORD GetCreationFlags() const noexcept;
			virtual DWORD GetProcessExitCode() const;

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