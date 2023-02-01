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
			Semaphore(const Semaphore& other);
			Semaphore(Semaphore&& other) noexcept;
			Semaphore(
				const bool isInheritable,
				const ULONG initialCount,
				const ULONG maxCount);
			Semaphore(
				std::wstring name,
				const bool isInheritable,
				const ULONG initialCount,
				const ULONG maxCount);
			Semaphore(
				std::wstring name,
				const bool isInheritable,
				const long initialCount,
				const long maxCount,
				const DWORD desiredAccess);
			
		public:
			virtual void operator=(const Semaphore& other);
			virtual void operator=(Semaphore&& other) noexcept;
			virtual operator bool() const noexcept;

		public:
			virtual void Close();
			virtual long Release();
			virtual long Release(const long countToRelease);
			virtual bool Acquire();
			virtual bool Acquire(const DWORD millisTimeout);
			virtual bool AcquireMany(const long countToAcquire, const DWORD millisTimeout);
			virtual bool Acquire(const DWORD millisTimeout, const bool isAlertable);
			virtual const std::wstring& GetName() const noexcept final;
			virtual long GetCurrentCount() const noexcept final;
			virtual long GetMaxCount() const noexcept final;
			virtual HANDLE GetHandle() const noexcept final;

		protected:
			virtual void Copy(const Semaphore& other);
			virtual void Move(Semaphore& other) noexcept;
			virtual void InternalCreate(
				const std::wstring& name,
				const ULONG initialCount,
				const ULONG maxCount,
				const bool isInheritable
			);

		protected:
			RAII::Win32Handle m_handle;
			std::wstring m_name;
			std::atomic<long> m_currentCount = 0;
			long m_maxCount = 0;
	};
}