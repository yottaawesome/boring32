#pragma once
#include <Windows.h>
#include <string>

namespace Win32Utils::Async
{
	class Process
	{
		public:
			virtual ~Process();
			Process();
			Process(
				const std::wstring& executablePath,
				const std::wstring& commandLine,
				const std::wstring& startingDirectory,
				const bool canInheritHandles,
				const DWORD creationFlags
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

		protected:
			virtual void Duplicate(const Process& other);
			virtual void Move(Process& other) noexcept;

		protected:
			std::wstring m_executablePath;
			std::wstring m_commandLine;
			std::wstring m_startingDirectory;
			bool m_canInheritHandles;
			PROCESS_INFORMATION m_processInfo;
			DWORD m_creationFlags;
	};
}