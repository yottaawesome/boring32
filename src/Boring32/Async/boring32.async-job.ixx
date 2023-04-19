export module boring32.async:job;
import boring32.raii;
import <string>;
import <win32.hpp>;

export namespace Boring32::Async
{
	class Job
	{
		// Constructors and destructor
		public:
			virtual ~Job() = default;
			Job() = default;
			Job(const Job& other) = default;
			Job(Job&& other) noexcept = default;
			Job(const bool isInheritable);
			Job(const bool isInheritable, std::wstring name);
			Job(const bool isInheritable, std::wstring name, const DWORD desiredAccess);

		public:
			virtual Job& operator=(const Job& other) = default;
			virtual Job& operator=(Job&& other) noexcept = default;

		// API
		public:
			virtual void SetInformation(JOBOBJECT_EXTENDED_LIMIT_INFORMATION& jeli);
			virtual void AssignProcessToThisJob(const HANDLE process);
			virtual HANDLE GetHandle() const noexcept;
			virtual const std::wstring& GetName() const noexcept;
			virtual void Close();
			virtual bool IsInheritable() const noexcept;

		protected:
			virtual void Create(const bool isInheritable);
			virtual void Open(const bool isInheritable);

		protected:
			RAII::Win32Handle m_job;
			std::wstring m_name;
	};
}