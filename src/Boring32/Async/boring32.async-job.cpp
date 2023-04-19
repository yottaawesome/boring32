module boring32.async:job;
import boring32.error;

namespace Boring32::Async
{
	void Job::Close()
	{
		m_job = nullptr;
	}

	Job::Job(const bool isInheritable)
	:	m_name(L"")
	{ 
		Create(isInheritable);
	}

	Job::Job(const bool isInheritable, std::wstring name)
	:	m_name(std::move(name))
	{
		Create(isInheritable);
	}
	
	Job::Job(const bool isInheritable, std::wstring name, const DWORD desiredAccess)
	:	m_name(std::move(name))
	{
		Open(isInheritable);
	}

	void Job::SetInformation(JOBOBJECT_EXTENDED_LIMIT_INFORMATION& jeli)
	{
		// See https://docs.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-jobobject_basic_limit_information
		// jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

		const bool succeeded = SetInformationJobObject(
			m_job.GetHandle(),
			JobObjectExtendedLimitInformation,
			&jeli,
			sizeof(jeli)
		);
		if (!succeeded)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error(
				"SetInformationJobObject() failed",
				lastError
			);
		}
	}

	void Job::AssignProcessToThisJob(const HANDLE process)
	{
		if (!m_job)
			throw Error::Boring32Error("Cannot assign process to job; job is not initialised");
		if (!process)
			throw Error::Boring32Error("Cannot assign process to job; process is null.");

		if (!AssignProcessToJobObject(m_job.GetHandle(), process))
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error(
				"Cannot assign process to job; AssignProcessToJobObject() failed.",
				lastError
			);
		}			
	}

	HANDLE Job::GetHandle() const noexcept
	{
		return m_job.GetHandle();
	}

	const std::wstring& Job::GetName() const noexcept
	{
		return m_name;
	}
	
	bool Job::IsInheritable() const noexcept
	{
		return m_job.IsInheritable();
	}

	void Job::Open(const bool isInheritable)
	{
		m_job = OpenJobObjectW(
			JOB_OBJECT_ALL_ACCESS,
			isInheritable,
			m_name.c_str()
		);
		if (!m_job)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error(
				"OpenJobObjectW() failed", 
				lastError
			);
		}
	}

	void Job::Create(const bool isInheritable)
	{
		m_job = CreateJobObjectW(
			nullptr, 
			m_name.empty() ? nullptr : m_name.c_str()
		);
		if (!m_job)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error(
				"CreateJobObjectW() failed",
				lastError
			);
		}
		m_job.SetInheritability(isInheritable);
	}
}