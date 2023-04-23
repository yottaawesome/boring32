export module boring32.async:semaphore;
import boring32.raii;
import <string>;
import <win32.hpp>;

export namespace Boring32::Async
{
	class Semaphore
	{
		public:
			virtual ~Semaphore() = default;
			Semaphore() = default;
			Semaphore(const Semaphore& other) = default;
			Semaphore(Semaphore&& other) noexcept = default;
			Semaphore(
				const bool isInheritable,
				const unsigned long initialCount,
				const unsigned long maxCount);
			Semaphore(
				std::wstring name,
				const bool isInheritable,
				const unsigned long initialCount,
				const unsigned long maxCount);
			Semaphore(
				std::wstring name,
				const bool isInheritable,
				const long initialCount,
				const long maxCount,
				const unsigned long desiredAccess);
			
		public:
			virtual Semaphore& operator=(const Semaphore& other) = default;
			virtual Semaphore& operator=(Semaphore&& other) noexcept = default;
			virtual operator bool() const noexcept;

		public:
			virtual void Close();
			virtual void Release();
			virtual void Release(const long countToRelease);
			virtual bool Acquire();
			virtual bool Acquire(const unsigned long millisTimeout);
			virtual bool AcquireMany(
				const long countToAcquire, 
				const unsigned long millisTimeout
			);
			virtual bool Acquire(const unsigned long millisTimeout, const bool isAlertable);
			virtual const std::wstring& GetName() const noexcept final;
			virtual long GetMaxCount() const noexcept final;
			virtual HANDLE GetHandle() const noexcept final;

		protected:
			virtual void InternalCreate(
				const std::wstring& name,
				const unsigned long initialCount,
				const unsigned long maxCount,
				const bool isInheritable
			);

		protected:
			RAII::Win32Handle m_handle;
			std::wstring m_name;
			long m_maxCount = 0;
	};
}