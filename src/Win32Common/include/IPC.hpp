#pragma once
#include <Windows.h>
#include <string>
#include <vector>

namespace Win32Utils::IPC
{
	class ProcessInfo
	{
		public:
			ProcessInfo();
			virtual ~ProcessInfo();
			virtual PROCESS_INFORMATION& GetProcessInfo();
			virtual PROCESS_INFORMATION* operator&();
			virtual HANDLE GetProcessHandle();
			virtual HANDLE GetThreadHandle();

		protected:
			PROCESS_INFORMATION m_processInfo;
	};

	class Pipe
	{
		// Constructors
		public:
			virtual ~Pipe();
			Pipe();
			Pipe(const Pipe& other);
			Pipe(Pipe&& other) noexcept;
			Pipe(const bool inheritable, const DWORD size, const std::wstring& delimiter);
			Pipe(
				const bool inheritable,
				const DWORD size, 
				const bool duplicate,
				const std::wstring& delimiter,
				const HANDLE readHandle, 
				const HANDLE writeHandle
			);
		
		// API
		public:
			virtual void Write(const std::wstring& msg);
			virtual std::wstring Read();
			virtual std::vector<std::wstring> DelimitedRead();
			virtual void CloseRead();
			virtual void CloseWrite();
			virtual HANDLE GetRead();
			virtual HANDLE GetWrite();

		// Overloaded operators
		public:
			virtual void operator=(const Pipe& other);
			virtual void operator=(Pipe&& other) noexcept;

		// Internal methods
		protected:
			virtual void Duplicate(const Pipe& other);
			virtual void Duplicate(const HANDLE readHandle, const HANDLE writeHandle);
			virtual void Cleanup();

		// Internal variables
		protected:
			std::wstring m_delimiter;
			DWORD m_size;
			bool m_inheritable;
			HANDLE m_readHandle;
			HANDLE m_writeHandle;
	};

	/// <summary>
	///		Represents a <a href="https://docs.microsoft.com/en-us/dotnet/standard/io/memory-mapped-files">Win32 memory-mapped file</a>,
	///		which is used for allowing process to share memory.
	/// </summary>
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
			/// <param name="createFile">
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

			/// <summary>
			///		Duplicates the specified MemoryMappedFile.
			/// </summary>
			/// <param name="other">The MemoryMappedFile to duplicate.</param>
			MemoryMappedFile(const MemoryMappedFile& other);

			/// <summary>
			///		Unmaps the view of the MemoryMappedFile and releases
			///		the handle owned by this object.
			/// </summary>
			virtual ~MemoryMappedFile();
			
			/// <summary>
			///		Gets the view pointer of the memory mapped file.
			/// </summary>
			/// <returns>The view object.</returns>
			virtual void* GetViewPointer();

			/// <summary>
			///		Unmaps the view of the MemoryMappedFile and releases
			///		the handle owned by this object before duplicating
			///		from the specified MemoryMappedFile.
			/// </summary>
			/// <param name="other">The MemoryMappedFile to duplicate.</param>
			virtual void operator=(const MemoryMappedFile& other);

		public:
			MemoryMappedFile() = delete;

		protected:
			/// <summary>
			///		Unlocks the mutex if it is currently owned, releases the 
			///		mutex handle, and clears the underlying pointer.
			/// </summary>
			virtual void Cleanup();

			/// <summary>
			///		Duplicates the specified MemoryMappedFile.
			/// </summary>
			/// <param name="other">The MemoryMappedFile to duplicate.</param>
			virtual void Duplicate(const MemoryMappedFile& other);

		protected:
			bool m_inheritable;
			std::wstring m_mmfName;
			UINT m_maxSize;
			HANDLE m_mapFile;
			void* m_View;
	};

	template<typename T>
	class MemoryMappedView
	{
		public:
			MemoryMappedView(const std::wstring& name, const bool create, const bool inheritable)
			:	m_mappedMemory(
					name,
					sizeof(T),
					create,
					inheritable
				),
				m_view(nullptr)
			{
				m_view = (T*) m_mappedMemory.GetViewPointer();
				if (create)
					m_view = new (m_view) T();
			}

			T* GetView()
			{
				return m_view;
			}

		protected:
			MemoryMappedFile m_mappedMemory;
			T* m_view;
	};

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
			///		Creates a new named or anonymous mutex, or opens an existing named one.
			/// </summary>
			/// <param name="name">
			///		The name of the mutex to create or open. Pass an empty
			///		string to create an anonymous mutex.
			/// </param>
			/// <param name="createNew">
			///		Whether to create a new mutex or open an existing one.
			/// </param>
			/// <param name="acquireOnCreation">
			///		Whether to acquire the mutex if it's being created.
			///		If the createNew parameter is false, this parameter is ignored.
			/// </param>
			/// <param name="inheritable">
			///		Whether the handle can be inherited by child processes.
			/// </param>
			Mutex(
				const std::wstring_view name,
				const bool createNew,
				const bool acquireOnCreation,
				const bool inheritable
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
			virtual void operator=(const Mutex& other);

			/// <summary>
			///		Move assignment.
			/// </summary>
			/// <param name="other">The mutex to move.</param>
			virtual void operator=(Mutex&& other) noexcept;

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

		protected:
			virtual void Cleanup();
			virtual void Move(Mutex& other) noexcept;

		protected:
			std::wstring m_name;
			bool m_inheritable;
			bool m_created;
			bool m_locked;
			HANDLE m_mutex;
	};

	class Process
	{
		public:
			virtual ~Process();
			Process();
			Process(
				const std::wstring& executablePath,
				const std::wstring& commandLine,
				const std::wstring& startingDirectory,
				const bool canInheritHandles,
				const DWORD creationFlags
			);

			// Move
			Process(Process&& other) noexcept;
			virtual void operator=(Process&& other) noexcept;

			// Copy
			Process(const Process& other);
			virtual void operator=(Process& other);

			virtual void Start();
			virtual void CloseHandles();
			virtual void CloseProcessHandle();
			virtual void CloseThreadHandle();

			virtual HANDLE GetProcessHandle();
			virtual HANDLE GetThreadHandle();

		protected:
			virtual void Duplicate(const Process& other);
			virtual void Move(Process& other);

		protected:
			std::wstring m_executablePath;
			std::wstring m_commandLine;
			std::wstring m_startingDirectory;
			bool m_canInheritHandles;
			PROCESS_INFORMATION m_processInfo;
			DWORD m_creationFlags;
	};
}
