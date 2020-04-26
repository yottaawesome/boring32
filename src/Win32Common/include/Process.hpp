#pragma once
#include <Windows.h>
#include <string>

namespace Win32Utils::Process
{
	class DynamicLinkLibrary
	{
		public:
			DynamicLinkLibrary(const std::wstring& path);
			virtual ~DynamicLinkLibrary();
			virtual const std::wstring& GetPath();
			virtual const HMODULE GetHandle();
			virtual void* Resolve(const std::wstring& path);

		protected:
			const std::wstring path;
			HMODULE libraryHandle;
	};

	class MemoryMappedFile
	{
		public:
			/// <summary>
			///		Creates or opens a new memory mapped file.
			/// </summary>
			/// <param name="name">
			///		The name of the memory mapped file to create or open.
			/// </param>
			/// <param name="maxSize">
			///		The maximum size of the memory mapped file.
			/// </param>
			/// <param name="createNewMutex">
			///		Whether to create a new memory mapped file or open an
			///		existing one that has been inherited.
			/// </param>
			/// <param name="inheritable">
			///		Whether the acquired handle can be inherited by child processes.
			/// </param>
			MemoryMappedFile(
				const std::wstring& name,
				const UINT maxSize,
				const bool createFile,
				const bool inheritable
			);
			virtual ~MemoryMappedFile();
			virtual void* GetViewPointer();
			virtual bool Initialised();

		public:
			MemoryMappedFile(const MemoryMappedFile& other) = delete;
			virtual void operator=(const MemoryMappedFile& other) = delete;

		protected:
			virtual void Cleanup();

		protected:
			bool m_initialised;
			bool m_createFile;
			bool m_inheritable;
			const std::wstring m_mmfName;
			const UINT m_maxSize;
			HANDLE m_MapFile;
			void* m_View;
	};

	/// <summary>
	///		Represents a Win32 mutex, an object used primarily for interprocess
	///		synchronisation and communication.
	/// </summary>
	class Mutex
	{
		public:
			/// <summary>
			///		Creates a new mutex, or opens an existing mutex.
			/// </summary>
			/// <param name="name">
			///		The name of the mutex to create or open.
			/// </param>
			/// <param name="create">
			///		Whether to create a new mutex or open an existing one.
			/// </param>
			/// <param name="acquire">
			///		Whether to acquire the mutex if it's being created.
			///		If the create parameter is false, this parameter is ignored.
			/// </param>
			/// <param name="inheritable">
			///		Whether the handle can be inherited by child processes.
			/// </param>
			Mutex(const std::wstring_view name, const bool create, const bool acquire, const bool inheritable);
			
			/// <summary>
			///		Creates an anymous mutex.
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
			/// Clones a mutex.
			/// </summary>
			/// <param name="other"></param>
			Mutex(const Mutex& other);

			/// <summary>
			/// Destroys this mutex.
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
			virtual void operator=(const Mutex& other);

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
			///		Frees the mutex, allowing another process to acquire it.
			/// <exception cref="std::runtime_error">
			///		Failed to release the mutex.
			/// </exception>
			/// </summary>
			virtual void Unlock();
	
		public:
			Mutex() = delete;

		protected:
			virtual void Cleanup();

		protected:
			std::wstring m_name;
			bool m_inheritable;
			bool m_created;
			bool m_locked;
			HANDLE m_mutex;
	};
}