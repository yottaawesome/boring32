module;

#include <string>
#include <vector>
#include <memory>
#include <Windows.h>

export module boring32.computer:processinfo;
import boring32.raii;

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
			virtual ~ProcessInfo();
			ProcessInfo(const ProcessInfo&);
			ProcessInfo(ProcessInfo&&) noexcept;
			ProcessInfo(HANDLE hProcess);

		public:
			virtual ProcessInfo& operator=(ProcessInfo&);
			virtual ProcessInfo& operator=(ProcessInfo&&) noexcept;

		public:
			virtual ProcessTimes GetTimes() const;
			virtual std::wstring GetPath() const;
			virtual DWORD GetID() const;
			virtual DWORD GetHandleCount() const;

		public:
			static std::vector<ProcessInfo> FromCurrentProcesses();

		protected:
			Raii::Win32Handle m_processHandle;
	};
}