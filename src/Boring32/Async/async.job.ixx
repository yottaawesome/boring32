export module boring32:async.job;
import std;
import :win32;
import :raii;
import :error;

export namespace Boring32::Async
{
	class Job final
	{
	public:
		Job() = default;
		Job(const Job& other) = default;
		Job(Job&& other) noexcept = default;

		Job(bool isInheritable)
		{
			Create(isInheritable);
		}

		Job(bool isInheritable, std::wstring name)
			: m_name(std::move(name))
		{
			Create(isInheritable);
		}

		Job(bool isInheritable, std::wstring name, Win32::DWORD desiredAccess)
			: m_name(std::move(name))
		{
			Open(isInheritable);
		}

		void SetInformation(const Win32::JOBOBJECT_EXTENDED_LIMIT_INFORMATION& jeli)
		{
			if (not m_job)
				throw Error::Boring32Error("Cannot assign process to job; job is not initialised");
			// See https://docs.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-jobobject_basic_limit_information
			// jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
			bool succeeded = Win32::SetInformationJobObject(
				m_job.GetHandle(),
				Win32::JOBOBJECTINFOCLASS::JobObjectExtendedLimitInformation,
				const_cast<Win32::JOBOBJECT_EXTENDED_LIMIT_INFORMATION*>(&jeli),
				sizeof(jeli)
			);
			if (not succeeded)
				throw Error::Win32Error{Win32::GetLastError(), "SetInformationJobObject() failed"};
		}

		void AssignProcessToThisJob(Win32::HANDLE process)
		{
			if (not m_job)
				throw Error::Boring32Error("Cannot assign process to job; job is not initialised");
			if (not process)
				throw Error::Boring32Error("Cannot assign process to job; process is null.");
			if (not Win32::AssignProcessToJobObject(m_job.GetHandle(), process))
				throw Error::Win32Error{Win32::GetLastError(), "Cannot assign process to job; AssignProcessToJobObject() failed."};
		}

		auto GetHandle() const noexcept -> Win32::HANDLE
		{
			return m_job.GetHandle();
		}

		auto GetName() const noexcept -> std::wstring
		{
			return m_name;
		}

		void Close()
		{
			m_job = nullptr;
		}

		auto IsInheritable() const noexcept -> bool
		{
			return m_job.IsInheritable();
		}

		operator bool() const noexcept
		{
			return m_job != nullptr;
		}

	private:
		void Create(bool isInheritable)
		{
			m_job = Win32::CreateJobObjectW(nullptr, m_name.empty() ? nullptr : m_name.c_str());
			if (not m_job)
				throw Error::Win32Error{Win32::GetLastError(), "CreateJobObjectW() failed"};
			m_job.SetInheritability(isInheritable);
		}

		void Open(const bool isInheritable)
		{
			m_job = Win32::OpenJobObjectW(Win32::JobObjectAllAccess, isInheritable, m_name.c_str());
			if (not m_job)
				throw Error::Win32Error{Win32::GetLastError(), "OpenJobObjectW() failed"};
		}

		RAII::Win32Handle m_job;
		std::wstring m_name;
	};
}