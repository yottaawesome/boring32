export module boring32.computer:processinfo;
import boring32.raii;
import <string>;
import <vector>;
import <memory>;
import <win32.hpp>;

export namespace Boring32::Computer
{
	struct ProcessTimes
	{
		size_t CreationTime = 0;
		size_t ExitTime = 0;
		size_t KernelTime = 0;
		size_t UserTime = 0;
		size_t ProcessTime = 0;
	};

	class ProcessInfo
	{
		public:
			virtual ~ProcessInfo() = default;
			ProcessInfo(const ProcessInfo&) = default;
			ProcessInfo(ProcessInfo&&) noexcept = default;
			ProcessInfo(const HANDLE hProcess);
			ProcessInfo(const DWORD processId);

		public:
			virtual ProcessInfo& operator=(ProcessInfo&) = default;
			virtual ProcessInfo& operator=(ProcessInfo&&) noexcept = default;

		public:
			virtual ProcessTimes GetTimes() const;
			virtual std::wstring GetPath() const;
			virtual DWORD GetID() const;
			virtual DWORD GetHandleCount() const;
			virtual DWORD GetExitCode() const;

		public:
			static std::vector<ProcessInfo> FromCurrentProcesses();

		protected:
			RAII::Win32Handle m_processHandle;
	};
}