export module boring32.async:mutex;
import boring32.raii;
import <string>;
import <win32.hpp>;

export namespace Boring32::Async
{
	/// <summary>
	///		Represents a Win32 mutex, an object used for interprocess
	///		synchronisation and communication.
	/// </summary>
	class Mutex
	{
		public:
			/// <summary>
			/// Default constructor. Does not initialise any underlying mutex.
			/// </summary>
			Mutex();

			/// <summary>
			///		Clones a mutex.
			/// </summary>
			/// <param name="other"></param>
			Mutex(const Mutex& other);

			/// <summary>
			///		Move constructor.
			/// </summary>
			/// <param name="other">The rvalue to move.</param>
			Mutex(Mutex&& other) noexcept;

			/// <summary>
			///		Destroys this mutex.
			/// </summary>
			virtual ~Mutex();

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
			Mutex(const bool acquire, const bool inheritable);

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
			);

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
				const bool inheritable,
				std::wstring name,
				const DWORD desiredAccess
			);

		public:
			/// <summary>
			///		Copy assignment.
			/// </summary>
			virtual Mutex& operator=(const Mutex& other);

			/// <summary>
			///		Move assignment.
			/// </summary>
			virtual Mutex& operator=(Mutex&& other) noexcept;
		
		public:
			virtual bool Lock();
			virtual bool Lock(const DWORD waitTime);
			virtual bool Lock(const bool isAlertable);

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
			virtual bool Lock(const DWORD waitTime, const bool isAlertable);

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
			virtual bool Lock(
				const DWORD waitTime, 
				const bool isAlertable, 
				const std::nothrow_t&
			) noexcept;

			/// <summary>
			///		Frees the mutex, allowing another process to acquire it.
			/// </summary>
			/// <exception cref="Error::Win32Error">
			///		Failed to release the mutex.
			/// </exception>
			virtual void Unlock();

			/// <summary>
			///		Frees the mutex, allowing another process to acquire it.
			///		Does not throw exceptions on failure.
			/// </summary>
			virtual bool Unlock(const std::nothrow_t&) noexcept;

			/// <summary>
			///		Invalidates and closes the native Mutex handle.
			/// </summary>
			virtual void Close();

			/// <summary>
			///		Retrieves this Mutex's underlying handle.
			/// </summary>
			/// <returns>This Mutex's underlying handle</returns>
			virtual HANDLE GetHandle() const noexcept;

			/// <summary>
			///		Retrieves this Mutex's name. This value is an empty string
			///		if this is an anonymous Mutex.
			/// </summary>
			/// <returns>This Mutex's name.</returns>
			virtual const std::wstring& GetName() const noexcept;

		protected:
			virtual void Move(Mutex& other) noexcept;
			virtual void Copy(const Mutex& other);

		protected:
			std::wstring m_name;
			bool m_created;
			bool m_locked;
			RAII::Win32Handle m_mutex;
	};
}