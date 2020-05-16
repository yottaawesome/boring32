#pragma once
#include <Windows.h>
#include <string>
#include "../Raii/Raii.hpp"

namespace Boring32::Async
{
	class Job
	{
		public:
			virtual ~Job();
			Job();
			Job(const bool isInheritable);
			Job(const bool isInheritable, const std::wstring name);

			Job(const Job& other);
			virtual void operator=(const Job& other);

			Job(Job&& other) noexcept;
			virtual void operator=(Job&& other) noexcept;

			virtual void SetInformation(JOBOBJECT_EXTENDED_LIMIT_INFORMATION& jeli);
			virtual void AssignProcessToThisJob(const HANDLE process);
			virtual HANDLE GetHandle();
			virtual std::wstring GetName() const;
			virtual void Close();
			virtual bool IsInheritable() const;

		protected:
			virtual void Create(const bool isInheritable);
			virtual void Copy(const Job& other);
			virtual void Move(Job& other) noexcept;

		protected:
			Raii::Win32Handle m_job;
			std::wstring m_name;
	};
}