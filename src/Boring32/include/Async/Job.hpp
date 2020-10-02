#pragma once
#include <Windows.h>
#include <string>
#include "../Raii/Raii.hpp"

namespace Boring32::Async
{
	class Job
	{
		// Constructors and destructor
		public:
			virtual ~Job();
			Job();
			Job(const bool isInheritable);
			Job(const bool isInheritable, std::wstring name);
			Job(const bool isInheritable, std::wstring name, const DWORD desiredAccess);

			Job(const Job& other);
			virtual Job& operator=(const Job& other);

			Job(Job&& other) noexcept;
			virtual Job& operator=(Job&& other) noexcept;

		// API
		public:
			virtual void SetInformation(JOBOBJECT_EXTENDED_LIMIT_INFORMATION& jeli);
			virtual void AssignProcessToThisJob(const HANDLE process);
			virtual HANDLE GetHandle();
			virtual const std::wstring& GetName() const;
			virtual void Close();
			virtual bool IsInheritable() const;

		protected:
			virtual void Create(const bool isInheritable);
			virtual void Open(const bool isInheritable);
			virtual void Copy(const Job& other);
			virtual void Move(Job& other) noexcept;

		protected:
			Raii::Win32Handle m_job;
			std::wstring m_name;
	};
}