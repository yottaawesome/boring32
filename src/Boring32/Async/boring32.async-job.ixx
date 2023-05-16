module;

#include <source_location>;

export module boring32.async:job;
import <string>;
import <win32.hpp>;
import boring32.raii;
import boring32.error;

export namespace Boring32::Async
{
	class Job final
	{
		// The Six
		public:
			~Job() = default;

			Job() = default;

			Job(const Job& other) = default;
			Job& operator=(const Job& other) = default;

			Job(Job&& other) noexcept = default;
			Job& operator=(Job&& other) noexcept = default;

		// Custom constructors
		public:
			Job(const bool isInheritable)
			{
				Create(isInheritable);
			}

			Job(const bool isInheritable, std::wstring name)
				: m_name(std::move(name))
			{
				Create(isInheritable);
			}

			Job(const bool isInheritable, std::wstring name, const DWORD desiredAccess)
				: m_name(std::move(name))
			{
				Open(isInheritable);
			}

		// API
		public:
			void SetInformation(JOBOBJECT_EXTENDED_LIMIT_INFORMATION& jeli)
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

			void AssignProcessToThisJob(const HANDLE process)
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

			HANDLE GetHandle() const noexcept
			{
				return m_job.GetHandle();
			}

			const std::wstring& GetName() const noexcept
			{
				return m_name;
			}

			void Close()
			{
				m_job = nullptr;
			}

			bool IsInheritable() const noexcept
			{
				return m_job.IsInheritable();
			}

		private:
			void Create(const bool isInheritable)
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

			void Open(const bool isInheritable)
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

		private:
			RAII::Win32Handle m_job;
			std::wstring m_name;
	};
}