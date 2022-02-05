module;

#include <Windows.h>
#include <string>

export module boring32.async.process;
import boring32.raii.win32handle;

export namespace Boring32::Async
{
	class Process
	{
		public:
			virtual ~Process();
			Process();
			Process(Process&& other) noexcept;
			Process(const Process& other);
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
			virtual Process& operator=(Process&& other) noexcept;
			virtual Process& operator=(Process& other);
		
		public:
			virtual void Start();
			virtual void CloseHandles();
			virtual void CloseProcessHandle();
			virtual void CloseThreadHandle();

			virtual HANDLE GetProcessHandle() const noexcept;
			virtual HANDLE GetThreadHandle() const noexcept;
			virtual std::wstring GetExecutablePath() const noexcept;
			virtual std::wstring GetCommandLineStr() const noexcept;
			virtual std::wstring GetStartingDirectory() const noexcept;
			virtual bool GetHandlesInheritability() const noexcept;
			virtual DWORD GetCreationFlags() const noexcept;

			virtual DWORD GetProcessExitCode() const;

		protected:
			virtual Process& Copy(const Process& other);
			virtual Process& Move(Process& other) noexcept;

		protected:
			std::wstring m_executablePath;
			std::wstring m_commandLine;
			std::wstring m_startingDirectory;
			bool m_canInheritHandles;
			DWORD m_creationFlags;
			Boring32::Raii::Win32Handle m_process;
			Boring32::Raii::Win32Handle m_thread;
			DWORD m_processId;
			DWORD m_threadId;
			STARTUPINFO m_dataSi;
	};
}