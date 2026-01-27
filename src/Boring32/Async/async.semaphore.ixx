export module boring32:async.semaphore;
import std;
import :win32;
import :error;
import :raii;
import :concepts;

export namespace Boring32::Async
{
	struct Semaphore final
	{
		Semaphore() = default;
		Semaphore(const Semaphore& other) = default;
		Semaphore(Semaphore&& other) noexcept = default;
		Semaphore& operator=(const Semaphore& other) = default;
		Semaphore& operator=(Semaphore&& other) noexcept = default;

		Semaphore(
			const bool isInheritable,
			const unsigned long initialCount,
			const unsigned long maxCount
		) : m_maxCount(maxCount)
		{
			InternalCreate(m_name, initialCount, maxCount, isInheritable);
		}

		Semaphore(
			std::wstring name,
			const bool isInheritable,
			const unsigned long initialCount,
			const unsigned long maxCount
		) : m_name(std::move(name)),
			m_maxCount(maxCount)
		{
			if (m_name.empty())
				throw Error::Boring32Error("Cannot create named semaphore with empty string.");
			InternalCreate(m_name, initialCount, maxCount, isInheritable);
		}

			Semaphore(
				std::wstring name,
				const bool isInheritable,
				const long initialCount,
				const long maxCount,
				const unsigned long desiredAccess
			) : m_name(std::move(name)),
				m_maxCount(maxCount)
			{
				if (initialCount > maxCount)
					throw Error::Boring32Error("Initial count exceeds maximum count.");
				if (m_name.empty())
					throw Error::Boring32Error("Cannot open semaphore with empty string.");
				if (maxCount == 0)
					throw Error::Boring32Error("MaxCount cannot be 0.");
				//SEMAPHORE_ALL_ACCESS
				m_handle = Win32::OpenSemaphoreW(desiredAccess, isInheritable, m_name.c_str());
				if (not m_handle)
					throw Error::Win32Error(Win32::GetLastError(), "failed to open semaphore");
			}
			
		operator bool() const noexcept
		{
			return m_handle != nullptr;
		}

		void Close()
		{
			m_handle = nullptr;
			m_name.clear();
		}

		void Release()
		{
			Release(1);
		}

		void Release(const long countToRelease)
		{
			if (countToRelease == 0)
				return;
			if (not m_handle)
				throw Error::Boring32Error("m_handle is nullptr.");

			long previousCount;
			if (not Win32::ReleaseSemaphore(*m_handle, countToRelease, &previousCount))
				throw Error::Win32Error(Win32::GetLastError(), "Failed to release semaphore");
		}

		bool Acquire()
		{
			return Acquire(Win32::Infinite, false);
		}

		bool Acquire(const unsigned long millisTimeout)
		{
			return Acquire(millisTimeout, false);
		}

		bool AcquireMany(const long countToAcquire, const unsigned long millisTimeout)
		{
			if (not m_handle)
				throw Error::Boring32Error("m_handle is nullptr.");
			if (countToAcquire > m_maxCount)
				throw Error::Boring32Error("Cannot acquire more than the maximum of the semaphore.");

			for (int actualAcquired = 0; actualAcquired < countToAcquire; actualAcquired++)
				if (not Acquire(millisTimeout))
				{
					Release(actualAcquired);
					return false;
				}

			return true;
		}

		bool Acquire(const Concepts::Duration auto& time, const bool isAlertable)
		{
			using std::chrono::milliseconds;
			using std::chrono::duration_cast;
			return Acquire(
				static_cast<unsigned long>(duration_cast<milliseconds>(time).count()), 
				isAlertable
			);
		}

		bool Acquire(const unsigned long millisTimeout, const bool isAlertable)
		{
			if (not m_handle)
				throw Error::Boring32Error("m_handle is nullptr.");

			const Win32::DWORD status = Win32::WaitForSingleObjectEx(
				*m_handle,
				millisTimeout,
				isAlertable
			);
			switch (status)
			{
				case Win32::WaitObject0:
					return true;

				case Win32::WaitTimeout:
					return false;

				case Win32::WaitAbandoned:
					throw Error::Boring32Error("The wait was abandoned.");

				case Win32::WaitFailed:
					throw Error::Win32Error(Win32::GetLastError(), "WaitForSingleObject() failed");

				default:
					throw Error::Boring32Error(
						"Unknown WaitForSingleObjectEx() value {}",
						std::source_location::current(),
						std::stacktrace::current(),
						status
					);
			}
		}

		const std::wstring& GetName() const noexcept
		{
			return m_name;
		}

		long GetMaxCount() const noexcept
		{
			return m_maxCount;
		}

		Win32::HANDLE GetHandle() const noexcept
		{
			return m_handle.GetHandle();
		}

		private:
		void InternalCreate(
			const std::wstring& name,
			const unsigned long initialCount,
			const unsigned long maxCount,
			const bool isInheritable
		)
		{
			if (initialCount > maxCount)
				throw Error::Boring32Error("Initial count exceeds maximum count.");
			if (maxCount == 0)
				throw Error::Boring32Error("MaxCount cannot be 0.");
			m_handle = Win32::CreateSemaphoreW(
				nullptr,
				initialCount,
				maxCount,
				name.empty() ? nullptr : name.c_str()
			);
			if (not m_handle)
				throw Error::Win32Error(Win32::GetLastError(), "Failed to create or open semaphore");

			m_handle.SetInheritability(isInheritable);
		}

		RAII::Win32Handle m_handle;
		std::wstring m_name;
		long m_maxCount = 0;
	};
}