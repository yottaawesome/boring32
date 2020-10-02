#pragma once
#include <Windows.h>
#include <string>
#include "../Raii/Raii.hpp"

namespace Boring32::Async
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
				std::wstring name,
				const bool acquireOnCreation,
				const bool inheritable
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
				std::wstring name,
				const DWORD desiredAccess,
				const bool isInheritable
			);
			
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
			///		Duplicates the mutex handle held by the RHS operand.
			/// </summary>
			/// <param name="other">
			///		The mutex whose handle needs to be duplicated.
			///		The handle owned by this mutex must not be null.
			/// </param>
			/// <exception cref="std::runtime_error">
			///		Thrown when the RHS operand is in an invalid state,
			///		or if the duplicatation failed.
			/// </exception>
			virtual Mutex& operator=(const Mutex& other);

			/// <summary>
			///		Move assignment.
			/// </summary>
			/// <param name="other">The mutex to move.</param>
			virtual Mutex& operator=(Mutex&& other) noexcept;

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
			/// <exception cref="std::runtime_error">
			///		Failed to acquire the mutex for reasons other than the
			///		timeout was reached.
			/// </exception>
			virtual bool Lock(const DWORD waitTime);

			/// <summary>
			///		Repeatedly tries to acquire a lock with a specified timeout.
			///		This allows threads to lock, but not be blocked indefinitely
			///		and unable to be forced to terminate.
			/// </summary>
			/// <returns>
			///		Returns true if the mutex was successfully acquired,
			///		or false if the timeout occurred.
			/// </returns>
			/// <exception cref="std::runtime_error">
			///		Failed to acquire the mutex for reasons other than the
			///		timeout was reached.
			/// </exception>
			virtual bool SafeLock(const DWORD waitTime, const DWORD sleepTime);

			/// <summary>
			///		Frees the mutex, allowing another process to acquire it.
			/// </summary>
			/// <exception cref="std::runtime_error">
			///		Failed to release the mutex.
			/// </exception>
			virtual void Unlock();

			/// <summary>
			///		Invalidates and closes the thread handle.
			/// </summary>
			virtual void Close();

		protected:
			virtual void Move(Mutex& other) noexcept;
			virtual void Copy(const Mutex& other);

		protected:
			std::wstring m_name;
			bool m_created;
			bool m_locked;
			Raii::Win32Handle m_mutex;
	};
}