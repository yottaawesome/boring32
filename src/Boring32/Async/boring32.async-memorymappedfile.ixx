export module boring32.async:memorymappedfile;
import std;

import <win32.hpp>;
import boring32.raii;
import boring32.error;

export namespace Boring32::Async
{
	/// <summary>
	///		Represents a <a href="https://docs.microsoft.com/en-us/dotnet/standard/io/memory-mapped-files">Win32 memory-mapped file</a>,
	///		which allows processes to share memory. This is a copyable
	///		and movable object.
	/// </summary>
	class MemoryMappedFile final
	{
		// The six
		public:
			/// <summary>
			///		Unmaps the view of the MemoryMappedFile and releases
			///		the handle owned by this object.
			/// </summary>
			~MemoryMappedFile()
			{
				Close(std::nothrow);
			}

			/// <summary>
			///		Default constructor. Does not initialise the MemoryMappedFile.
			/// </summary>
			MemoryMappedFile() = default;

			/// <summary>
			///		Duplicates the specified MemoryMappedFile.
			/// </summary>
			/// <param name="other">The MemoryMappedFile to duplicate.</param>
			MemoryMappedFile(const MemoryMappedFile& other)
				: m_name(other.m_name),
				m_maxSize(other.m_maxSize)
			{
				Copy(other);
			}

			/// <summary>
			///		Move constructor.
			/// </summary>
			MemoryMappedFile(MemoryMappedFile&& other) noexcept
			{
				Move(other);
			}

			/// <summary>
			///		Unmaps the view of the MemoryMappedFile and releases
			///		the handle owned by this object before duplicating
			///		from the specified MemoryMappedFile.
			/// </summary>
			MemoryMappedFile& operator=(const MemoryMappedFile& other)
			{
				Close();
				Copy(other);
				return *this;
			}

			/// <summary>
			///		Move assignment.
			/// </summary>
			MemoryMappedFile& operator=(MemoryMappedFile&& other) noexcept
			{
				Move(other);
				return *this;
			}

			// Custom constructors
		public:
			/// <summary>
			///		Creates a new memory mapped file.
			/// </summary>
			/// <param name="name">
			///		The name of the memory mapped file to create or open.
			/// </param>
			/// <param name="maxSize">
			///		The maximum size of the memory mapped file.
			/// </param>
			/// <param name="inheritable">
			///		Whether the acquired handle can be inherited by child processes.
			/// </param>
			MemoryMappedFile(
				std::wstring name,
				const UINT maxSize,
				const bool inheritable
			) : m_name(std::move(name)),
				m_maxSize(maxSize)
			{
				m_mapFile = CreateFileMappingW(
					INVALID_HANDLE_VALUE,		// use paging file
					nullptr,					// default security
					PAGE_READWRITE,				// read/write access
					0,							// maximum object size (high-order DWORD)
					m_maxSize,					// maximum object size (low-order DWORD)
					m_name.c_str()				// m_name of mapping object
				);
				if (!m_mapFile)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("CreateFileMappingW() failed", lastError);
				}

				m_mapFile.SetInheritability(inheritable);
				m_view = MapViewOfFile(
					m_mapFile.GetHandle(),	// handle to map object
					FILE_MAP_ALL_ACCESS,	// read/write permission
					0,
					0,
					maxSize
				);
				if (!m_view)
				{
					Close();
					const auto lastError = GetLastError();
					throw Error::Win32Error("MapViewOfFile() failed", lastError);
				}

				RtlSecureZeroMemory(m_view, m_maxSize);
			}

			/// <summary>
			///		Opens a memory mapped file.
			/// </summary>
			/// <param name="name">
			///		The name of the memory mapped file to create or open.
			/// </param>
			/// <param name="maxSize">
			///		The maximum size of the memory mapped file.
			/// </param>
			/// <param name="inheritable">
			///		Whether the acquired handle can be inherited by child processes.
			/// </param>
			/// <param name="desiredAccess">
			///		The desired access to open the file. See: https://docs.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-mapviewoffile
			/// </param>
			MemoryMappedFile(
				std::wstring name,
				const UINT maxSize,
				const bool inheritable,
				const DWORD desiredAccess
			) : m_name(std::move(name)),
				m_maxSize(maxSize)
			{
				// desiredAccess: https://docs.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-mapviewoffile
				m_mapFile = OpenFileMappingW(
					desiredAccess,	// read/write access
					inheritable,	// Should the handle be inheritable
					m_name.c_str()	// name of mapping object
				);
				if (!m_mapFile)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("OpenFileMappingW() failed", lastError);
				}

				m_view = MapViewOfFile(
					m_mapFile.GetHandle(),	// handle to map object
					desiredAccess,	// read/write permission
					0,
					0,
					maxSize
				);
				if (!m_view)
				{
					Close();
					const auto lastError = GetLastError();
					throw Error::Win32Error("MapViewOfFile() failed", lastError);
				}
			}

		// API
		public:
			/// <summary>
			///		Gets the view pointer of the memory mapped file.
			/// </summary>
			/// <returns>The view object.</returns>
			void* GetViewPointer() const noexcept
			{
				return m_view;
			}

			/// <summary>
			///		Get the name of this MemoryMappedFile.
			/// </summary>
			/// <returns>The name of this MemoryMappedFile.</returns>
			const std::wstring& GetName() const noexcept
			{
				return m_name;
			}

			/// <summary>
			///		Get whether this MemoryMappedFile can be inherited
			///		by child processes.
			/// </summary>
			/// <returns></returns>
			bool IsInheritable() const
			{
				return m_mapFile.IsInheritable();
			}

			/// <summary>
			///		Releases all resources owned by this object.
			/// </summary>
			void Close()
			{
				if (m_view && !UnmapViewOfFile(m_view))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("UnmapViewOfFile() failed", lastError);
				}
				m_view = nullptr;
				m_mapFile = nullptr;
			}

			/// <summary>
			///		Releases all resources owned by this object.
			/// </summary>
			void Close(const std::nothrow_t&) noexcept try
			{
				Close();
			}
			catch (const std::exception& ex)
			{
				std::wcerr << std::format("Close() failed: {}\n", ex.what()).c_str();
			}

		private:
			/// <summary>
			///		Moves the MemoryMappedFile on the RHS into this object.
			/// </summary>
			/// <param name="other">The MemoryMappedFile to move.</param>
			void Move(MemoryMappedFile& other) noexcept
			{
				Close();
				m_name = std::move(other.m_name);
				m_mapFile = std::move(other.m_mapFile);
				m_maxSize = other.m_maxSize;
				m_view = other.m_view;
				other.m_view = nullptr;
			}

			/// <summary>
			///		Duplicates the specified MemoryMappedFile.
			/// </summary>
			/// <param name="other">The MemoryMappedFile to duplicate.</param>
			void Copy(const MemoryMappedFile& other)
			{
				Close();
				m_name = other.m_name;
				m_maxSize = other.m_maxSize;
				m_mapFile = other.m_mapFile;
				if (!m_mapFile)
					return;

				m_view = MapViewOfFile(
					m_mapFile.GetHandle(),   // handle to map object
					FILE_MAP_ALL_ACCESS, // read/write permission
					0,
					0,
					m_maxSize
				);
				if (!m_view)
				{
					Close();
					const auto lastError = GetLastError();
					throw Error::Win32Error("MapViewOfFile() failed", lastError);
				}
			}

		private:
			std::wstring m_name;
			unsigned m_maxSize = 0;
			RAII::Win32Handle m_mapFile;
			void* m_view = nullptr;
	};
}