#pragma once
#include <Windows.h>
#include <string>
#include "../Raii/Raii.hpp"

namespace Boring32::Async
{
	class Process
	{
		public:
			virtual ~Process();
			Process();
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

			// Move
			Process(Process&& other) noexcept;
			virtual void operator=(Process&& other) noexcept;

			// Copy
			Process(const Process& other);
			virtual void operator=(Process& other);

			virtual void Start();
			virtual void CloseHandles();
			virtual void CloseProcessHandle();
			virtual void CloseThreadHandle();

			virtual HANDLE GetProcessHandle();
			virtual HANDLE GetThreadHandle();
			virtual std::wstring GetExecutablePath();
			virtual std::wstring GetCommandLineStr();
			virtual std::wstring GetStartingDirectory();
			virtual bool GetHandlesInheritability();
			virtual DWORD GetCreationFlags();

			virtual DWORD GetProcessExitCode();

		protected:
			virtual void Duplicate(const Process& other);
			virtual void Move(Process& other) noexcept;

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