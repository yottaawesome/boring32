export module boring32.async:semaphore;
import <string>;
import <stdexcept>;
import <format>;
import <win32.hpp>;
import boring32.error;
import boring32.raii;

export namespace Boring32::Async
{
	class Semaphore final
	{
		public:
			~Semaphore() = default;
			Semaphore() = default;
			Semaphore(const Semaphore& other) = default;
			Semaphore(Semaphore&& other) noexcept = default;
			Semaphore& operator=(const Semaphore& other) = default;
			Semaphore& operator=(Semaphore&& other) noexcept = default;

		public:
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
				m_handle = OpenSemaphoreW(desiredAccess, isInheritable, m_name.c_str());
				if (!m_handle)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("failed to open semaphore", lastError);
				}
			}
			
		public:
			operator bool() const noexcept
			{
				return m_handle != nullptr;
			}

		public:
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
				if (!m_handle)
					throw Error::Boring32Error("m_handle is nullptr.");

				long previousCount;
				if (!ReleaseSemaphore(*m_handle, countToRelease, &previousCount))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("Failed to release semaphore", lastError);
				}
			}

			bool Acquire()
			{
				return Acquire(INFINITE, false);
			}

			bool Acquire(const unsigned long millisTimeout)
			{
				return Acquire(millisTimeout, false);
			}

			bool AcquireMany(
				const long countToAcquire, 
				const unsigned long millisTimeout
			)
			{
				if (!m_handle)
					throw Error::Boring32Error("m_handle is nullptr.");
				if (countToAcquire > m_maxCount)
					throw Error::Boring32Error("Cannot acquire more than the maximum of the semaphore.");

				for (int actualAcquired = 0; actualAcquired < countToAcquire; actualAcquired++)
				{
					if (!Acquire(millisTimeout))
					{
						Release(actualAcquired);
						return false;
					}
				}

				return true;
			}

			bool Acquire(const unsigned long millisTimeout, const bool isAlertable)
			{
				if (!m_handle)
					throw Error::Boring32Error("m_handle is nullptr.");

				const DWORD status = WaitForSingleObjectEx(
					*m_handle,
					millisTimeout,
					isAlertable
				);
				switch (status)
				{
					case WAIT_OBJECT_0:
						return true;

					case WAIT_TIMEOUT:
						return false;

					case WAIT_ABANDONED:
						throw Error::Boring32Error("The wait was abandoned.");

					case WAIT_FAILED:
					{
						const auto lastError = GetLastError();
						throw Error::Win32Error("WaitForSingleObject() failed", lastError);
					}

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

			HANDLE GetHandle() const noexcept
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
				m_handle = CreateSemaphoreW(
					nullptr,
					initialCount,
					maxCount,
					name.empty() ? nullptr : name.c_str()
				);
				if (!m_handle)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error(
						"Failed to create or open semaphore",
						lastError
					);
				}

				m_handle.SetInheritability(isInheritable);
			}

		private:
			RAII::Win32Handle m_handle;
			std::wstring m_name;
			long m_maxCount = 0;
	};
}