module;

#include <Windows.h>

export module boring32.async.processinfo;

export namespace Boring32::Async
{
	class ProcessInfo
	{
		public:
			virtual ~ProcessInfo();
			ProcessInfo();
			ProcessInfo(const ProcessInfo& other);
			virtual void operator=(const ProcessInfo& other);
			ProcessInfo(ProcessInfo&& other) noexcept;
			virtual void operator=(ProcessInfo&& other) noexcept;

			virtual PROCESS_INFORMATION& GetProcessInfo();
			virtual PROCESS_INFORMATION* operator&();
			virtual HANDLE GetProcessHandle();
			virtual HANDLE GetThreadHandle();

		protected:
			virtual void Copy(const ProcessInfo& other);
			virtual void Move(ProcessInfo& other) noexcept;

		protected:
			PROCESS_INFORMATION m_processInfo;
	};
}