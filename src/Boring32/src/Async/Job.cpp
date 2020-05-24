#include "pch.hpp"
#include <stdexcept>
#include "include/Async/Job.hpp"

namespace Boring32::Async
{
	Job::~Job()
	{ 
		Close();
	}

	void Job::Close()
	{
		if (m_job != nullptr)
		{
			m_job.Close();
			m_job = nullptr;
		}
	}

	Job::Job()
	:	m_name(L"")
	{ }

	Job::Job(const bool isInheritable)
	:	m_name(L"")
	{ 
		Create(isInheritable);
	}

	Job::Job(const bool createOrOpen, const bool isInheritable, const std::wstring name)
	:	m_name(name)
	{
		if (createOrOpen)
			Create(isInheritable);
		else 
			Open(isInheritable);
	}

	Job::Job(const Job& other)
	{
		Copy(other);
	}

	void Job::operator=(const Job& other)
	{
		Close();
		Copy(other);
	}

	void Job::Copy(const Job& other)
	{
		m_name = other.m_name;
		if (other.m_job != nullptr)
			m_job = other.m_job;
	}

	Job::Job(Job&& other) noexcept
	{
		Move(other);
	}

	void Job::operator=(Job&& other) noexcept
	{
		Close();
		Move(other);
	}

	void Job::Move(Job& other) noexcept
	{
		m_name = std::move(other.m_name);
		if (other.m_job != nullptr)
			m_job = std::move(other.m_job);
	}

	void Job::SetInformation(JOBOBJECT_EXTENDED_LIMIT_INFORMATION& jeli)
	{
		// See https://docs.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-jobobject_basic_limit_information
		// jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

		bool bSuccess = SetInformationJobObject(
			m_job.GetHandle(),
			JobObjectExtendedLimitInformation,
			&jeli,
			sizeof(jeli)
		);
		if (bSuccess == false)
			throw std::runtime_error("SetInformationJobObject failed");
	}

	void Job::AssignProcessToThisJob(const HANDLE process)
	{
		if (m_job == nullptr)
			throw std::runtime_error("Cannot assign process to job; job is not initialised");
		if (process == nullptr)
			throw std::runtime_error("Cannot assign process to job; process is null.");

		if(AssignProcessToJobObject(m_job.GetHandle(), process) == false)
			throw std::runtime_error("Cannot assign process to job; AssignProcessToJobObject() failed.");
	}

	HANDLE Job::GetHandle()
	{
		return m_job.GetHandle();
	}

	std::wstring Job::GetName() const
	{
		return m_name;
	}
	
	bool Job::IsInheritable() const
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
		if (m_job == nullptr)
			throw std::runtime_error("CreateJobObject failed");
	}

	void Job::Create(const bool isInheritable)
	{
		SECURITY_ATTRIBUTES jobAttributes{ 0 };
		jobAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
		jobAttributes.bInheritHandle = isInheritable;

		m_job = CreateJobObject(
			&jobAttributes, 
			m_name.size() > 0 
				? m_name.c_str() 
				: nullptr
		);
		if (m_job == nullptr)
			throw std::runtime_error("CreateJobObject failed");
	}
}