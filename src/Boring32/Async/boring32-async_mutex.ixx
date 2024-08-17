export module boring32:async_mutex;
import boring32.shared;
import :error;
import :raii;
import :concepts;
import :async_functions;

export namespace Boring32::Async
{
	/// <summary>
	///		Represents a Win32 mutex, an object used for interprocess
	///		synchronisation and communication.
	/// </summary>
	class Mutex final
	{
		// The six
		public:
			/// <summary>
			/// Default constructor. Does not initialise any underlying mutex.
			/// </summary>
			Mutex() = default;

			/// <summary>
			///		Move constructor.
			/// </summary>
			/// <param name="other">The rvalue to move.</param>
			Mutex(Mutex&& other) noexcept
			{
				Move(other);
			}

			/// <summary>
			///		Move assignment.
			/// </summary>
			Mutex& operator=(Mutex&& other) noexcept
			{
				Move(other);
				return *this;
			}

			/// <summary>
			///		Destructor.
			/// </summary>
			~Mutex()
			{
				Close();
			}

			// Custom constructors
		public:
			/// <summary>
			///		Creates an anonymous mutex.
			/// </summary>
			/// <param name="acquire">
			///		Whether to request acquisition of the mutex.
			/// </param>
			/// <param name="inheritable">
			///		Whether the mutex is inheritable to child processes.
			/// </param>
			/// <exception cref="std::runtime_error">
			///		Thrown when mutex creation failed.
			/// </exception>
			Mutex(const bool acquire, const bool inheritable)
				: m_locked(acquire)
			{
				m_mutex = Win32::CreateMutexW(
					nullptr,
					m_locked,
					nullptr
				);
				m_mutex.SetInheritability(inheritable);
				if (!m_mutex)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("Failed to create mutex", lastError);
				}
			}

			/// <summary>
			///		Creates a new named or anonymous mutex.
			/// </summary>
			/// <param name="name">
			///		The name of the mutex to create. Pass an empty
			///		string to create an anonymous mutex.
			/// </param>
			/// <param name="acquireOnCreation">
			///		Whether to acquire the mutex on creation.
			/// </param>
			/// <param name="inheritable">
			///		Whether the handle can be inherited by child processes.
			/// </param>
			Mutex(
				const bool acquireOnCreation,
				const bool inheritable,
				std::wstring name
			) : m_name(std::move(name)),
				m_created(true)
			{
				m_mutex = Win32::CreateMutexW(
					nullptr,
					acquireOnCreation,
					m_name.empty() ? nullptr : m_name.c_str()
				);
				m_mutex.SetInheritability(inheritable);
				if (!m_mutex)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("Failed to create mutex", lastError);
				}

				m_locked = acquireOnCreation;
			}

			/// <summary>
			///		Opens an existing named mutex.
			/// </summary>
			/// <param name="inheritable">
			///		Whether the handle can be inherited by child processes.
			/// </param>
			/// <param name="name">
			///		The name of the mutex to open. Must not be the empty string.
			/// </param>
			/// <param name="desiredAccess">
			///		The desired access to open the mutex, e.g. SYNCHRONIZE.
			/// </param>
			Mutex(
				const bool acquireOnOpen,
				const bool isInheritable,
				std::wstring name,
				const Win32::DWORD desiredAccess
			) : m_name(name)
			{
				if (m_name.empty())
					throw Error::Boring32Error("Cannot open mutex with empty name");
				m_mutex = Win32::OpenMutexW(desiredAccess, isInheritable, m_name.c_str());
				if (!m_mutex)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("Failed to open mutex", lastError);
				}
				if (acquireOnOpen)
					Lock(Win32::Infinite, true);
			}

		// API
		public:
			bool Lock()
			{
				return Lock(Win32::Infinite, false);
			}

			bool Lock(const Win32::DWORD waitTime)
			{
				return Lock(waitTime, false);
			}

			bool Lock(const bool isAlertable)
			{
				return Lock(Win32::Infinite, isAlertable);
			}

			bool Lock(
				const Concepts::Duration auto& time,
				const bool alertable
			)
			{
				using std::chrono::duration_cast;
				using std::chrono::milliseconds;
				return Lock(
					static_cast<Win32::DWORD>(duration_cast<milliseconds>(time).count()),
					alertable
				);
			}

			/// <summary>
			///		Blocks the current thread for a specified amount of time 
			///		(or indefinitely) until the mutex is acquired.
			/// </summary>
			/// <param name="waitTime">
			///		The time in milliseconds to wait to acquire the mutex.
			///		Pass INFINITE to wait indefinitely.
			/// </param>
			/// <returns>
			///		Returns true if the mutex was successfully acquired,
			///		or false if the timeout occurred.
			/// </returns>
			/// <exception cref="Error::Boring32Error">
			///		Mutex not initialised.
			/// </exception>
			bool Lock(const Win32::DWORD waitTime, const bool isAlertable)
			{
				if (!m_mutex)
					throw Error::Boring32Error("Cannot wait on null mutex");
				return m_locked = WaitFor(
					m_mutex.GetHandle(), 
					waitTime, 
					isAlertable
				) == Win32::WaitResult::Success;
			}

			/// <summary>
			///		Blocks the current thread for a specified amount of time 
			///		(or indefinitely) until the mutex is acquired. Does not
			///		throw exceptions on failure.
			/// </summary>
			/// <param name="waitTime">
			///		The time in milliseconds to wait to acquire the mutex.
			///		Pass INFINITE to wait indefinitely.
			/// </param>
			/// <returns>
			///		Returns true if the mutex was successfully acquired,
			///		or false if the timeout occurred, or if an error occurred.
			/// </returns>
			bool Lock(
				const Win32::DWORD waitTime,
				const bool isAlertable, 
				const std::nothrow_t&
			) noexcept try
			{
				return Lock(waitTime, isAlertable);
			}
			catch (const std::exception& ex)
			{
				std::wcerr << std::format("Lock() failed: {}\n", ex.what()).c_str();
				return false;
			}

			/// <summary>
			///		Frees the mutex, allowing another process to acquire it.
			/// </summary>
			/// <exception cref="Error::Win32Error">
			///		Failed to release the mutex.
			/// </exception>
			void Unlock()
			{
				if (!m_mutex)
					throw Error::Boring32Error("Cannot wait on null mutex");
				if (!Win32::ReleaseMutex(m_mutex.GetHandle()))
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("Failed to release mutex", lastError);
				}

				m_locked = false;
			}

			/// <summary>
			///		Frees the mutex, allowing another process to acquire it.
			///		Does not throw exceptions on failure.
			/// </summary>
			bool Unlock(const std::nothrow_t&) noexcept try
			{
				Unlock();
				return true;
			}
			catch (const std::exception& ex)
			{
				std::wcerr << std::format("Unlock() failed: {}\n", ex.what()).c_str();
				return false;
			}

			/// <summary>
			///		Invalidates and closes the native Mutex handle.
			/// </summary>
			void Close()
			{
				if (!m_mutex)
					return;
				if (m_locked)
					Unlock();
				m_mutex.Close();
			}

			/// <summary>
			///		Retrieves this Mutex's underlying handle.
			/// </summary>
			/// <returns>This Mutex's underlying handle</returns>
			Win32::HANDLE GetHandle() const noexcept
			{
				return m_mutex.GetHandle();
			}

			/// <summary>
			///		Retrieves this Mutex's name. This value is an empty string
			///		if this is an anonymous Mutex.
			/// </summary>
			/// <returns>This Mutex's name.</returns>
			const std::wstring& GetName() const noexcept
			{
				return m_name;
			}

		private:
			void Move(Mutex& other) noexcept
			{
				Close();
				m_name = std::move(other.m_name);
				m_created = other.m_created;
				m_locked = other.m_locked.load(); // TODO: probably pointless, remove
				m_mutex = std::move(other.m_mutex);
			}

		private:
			std::wstring m_name;
			bool m_created = false;
			std::atomic<bool> m_locked = false;
			RAII::Win32Handle m_mutex;
	};
}