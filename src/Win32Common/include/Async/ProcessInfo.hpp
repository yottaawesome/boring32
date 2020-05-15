#pragma once
#include <Windows.h>

namespace Win32Utils::Async
{
	class ProcessInfo
	{
		public:
			ProcessInfo();
			virtual ~ProcessInfo();
			virtual PROCESS_INFORMATION& GetProcessInfo();
			virtual PROCESS_INFORMATION* operator&();
			virtual HANDLE GetProcessHandle();
			virtual HANDLE GetThreadHandle();

		protected:
			PROCESS_INFORMATION m_processInfo;
	};
}