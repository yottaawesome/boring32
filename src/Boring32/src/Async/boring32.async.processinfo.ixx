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
			ProcessInfo(ProcessInfo&& other) noexcept;

		public:
			virtual void operator=(const ProcessInfo& other);
			virtual void operator=(ProcessInfo&& other) noexcept;
		
		public:
			virtual PROCESS_INFORMATION& GetProcessInfo() noexcept;
			virtual const PROCESS_INFORMATION& GetProcessInfo() const noexcept;
			virtual PROCESS_INFORMATION* operator&() noexcept;
			virtual HANDLE GetProcessHandle() const noexcept;
			virtual HANDLE GetThreadHandle() const noexcept;

		protected:
			virtual void Copy(const ProcessInfo& other);
			virtual void Move(ProcessInfo& other) noexcept;

		protected:
			PROCESS_INFORMATION m_processInfo;
	};
}