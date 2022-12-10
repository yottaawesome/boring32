export module boring32.async:processinfo;
import <win32.hpp>;

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
			virtual ProcessInfo& operator=(const ProcessInfo& other);
			virtual ProcessInfo& operator=(ProcessInfo&& other) noexcept;
		
		public:
			virtual void Close();
			virtual PROCESS_INFORMATION& GetProcessInfo() noexcept;
			virtual const PROCESS_INFORMATION& GetProcessInfo() const noexcept;
			virtual PROCESS_INFORMATION* operator&() noexcept;
			virtual HANDLE GetProcessHandle() const noexcept;
			virtual HANDLE GetThreadHandle() const noexcept;

		protected:
			virtual ProcessInfo& Copy(const ProcessInfo& other);
			virtual ProcessInfo& Move(ProcessInfo& other) noexcept;

		protected:
			PROCESS_INFORMATION m_processInfo;
	};
}