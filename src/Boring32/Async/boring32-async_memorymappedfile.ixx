export module boring32:async_memorymappedfile;
import std;
import boring32.win32;
import :raii;
import :error;

export namespace Boring32::Async
{
	///	Represents a <a href="https://docs.microsoft.com/en-us/dotnet/standard/io/memory-mapped-files">Win32 memory-mapped file</a>,
	///	which allows processes to share memory. This is a copyable
	///	and movable object.
	struct MemoryMappedFile final
	{
		///	Unmaps the view of the MemoryMappedFile and releases
		///	the handle owned by this object.
		~MemoryMappedFile()
		{
			Close(std::nothrow);
		}

		///	Default constructor. Does not initialise the MemoryMappedFile.
		MemoryMappedFile() = default;

		///	Duplicates the specified MemoryMappedFile.
		MemoryMappedFile(const MemoryMappedFile& other)
			: m_name(other.m_name),
			m_maxSize(other.m_maxSize)
		{
			Copy(other);
		}

		///	Move constructor.
		MemoryMappedFile(MemoryMappedFile&& other) noexcept
		{
			Move(other);
		}

		///	Unmaps the view of the MemoryMappedFile and releases
		///	the handle owned by this object before duplicating
		///	from the specified MemoryMappedFile.
		MemoryMappedFile& operator=(const MemoryMappedFile& other)
		{
			Close();
			Copy(other);
			return *this;
		}

		///	Move assignment.
		MemoryMappedFile& operator=(MemoryMappedFile&& other) noexcept
		{
			Move(other);
			return *this;
		}

		///	Creates a new memory mapped file.
		MemoryMappedFile(std::wstring name, const unsigned maxSize, const bool inheritable) 
			: m_name(std::move(name)),
			m_maxSize(maxSize)
		{
			m_mapFile = Win32::CreateFileMappingW(
				Win32::InvalidHandleValue,		// use paging file
				nullptr,					// default security
				Win32::MemoryProtection::PageReadWrite,				// read/write access
				0,							// maximum object size (high-order DWORD)
				m_maxSize,					// maximum object size (low-order DWORD)
				m_name.c_str()				// m_name of mapping object
			);
			if (not m_mapFile)
				throw Error::Win32Error(Win32::GetLastError(), "CreateFileMappingW() failed");

			m_mapFile.SetInheritability(inheritable);
			m_view = Win32::MapViewOfFile(
				m_mapFile.GetHandle(),	// handle to map object
				static_cast<DWORD>(Win32::FileMapAccess::All),	// read/write permission
				0,
				0,
				maxSize
			);
			if (not m_view)
			{
				auto lastError = Win32::GetLastError();
				Close();
				throw Error::Win32Error(lastError, "MapViewOfFile() failed");
			}

			Win32::RtlSecureZeroMemory(m_view, m_maxSize);
		}

		///	Opens a memory mapped file.
		MemoryMappedFile(
			std::wstring name,
			const unsigned maxSize,
			const bool inheritable,
			const Win32::DWORD desiredAccess
		) : m_name(std::move(name)),
			m_maxSize(maxSize)
		{
			// desiredAccess: https://docs.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-mapviewoffile
			m_mapFile = Win32::OpenFileMappingW(
				desiredAccess,	// read/write access
				inheritable,	// Should the handle be inheritable
				m_name.c_str()	// name of mapping object
			);
			if (not m_mapFile)
				throw Error::Win32Error(Win32::GetLastError(), "OpenFileMappingW() failed");

			m_view = Win32::MapViewOfFile(
				m_mapFile.GetHandle(),	// handle to map object
				desiredAccess,	// read/write permission
				0,
				0,
				maxSize
			);
			if (not m_view)
			{
				auto lastError = Win32::GetLastError();
				Close();
				throw Error::Win32Error(lastError, "MapViewOfFile() failed");
			}
		}

		///	Gets the view pointer of the memory mapped file.
		void* GetViewPointer() const noexcept
		{
			return m_view;
		}

		///	Get the name of this MemoryMappedFile.
		const std::wstring& GetName() const noexcept
		{
			return m_name;
		}

		///	Get whether this MemoryMappedFile can be inherited
		///	by child processes.
		bool IsInheritable() const
		{
			return m_mapFile.IsInheritable();
		}

		///	Releases all resources owned by this object.
		void Close()
		{
			if (m_view && !Win32::UnmapViewOfFile(m_view))
				throw Error::Win32Error(Win32::GetLastError(), "UnmapViewOfFile() failed");
			m_view = nullptr;
			m_mapFile = nullptr;
		}

		///	Releases all resources owned by this object.
		void Close(const std::nothrow_t&) noexcept try
		{
			Close();
		}
		catch (const std::exception& ex)
		{
			std::wcerr << std::format("Close() failed: {}\n", ex.what()).c_str();
		}

		private:
		///	Moves the MemoryMappedFile on the RHS into this object.
		void Move(MemoryMappedFile& other) noexcept
		{
			Close();
			m_name = std::move(other.m_name);
			m_mapFile = std::move(other.m_mapFile);
			m_maxSize = other.m_maxSize;
			m_view = other.m_view;
			other.m_view = nullptr;
		}

		///	Duplicates the specified MemoryMappedFile.
		void Copy(const MemoryMappedFile& other)
		{
			Close();
			m_name = other.m_name;
			m_maxSize = other.m_maxSize;
			m_mapFile = other.m_mapFile;
			if (not m_mapFile)
				return;

			m_view = MapViewOfFile(
				m_mapFile.GetHandle(),   // handle to map object
				static_cast<Win32::DWORD>(Win32::FileMapAccess::All), // read/write permission
				0,
				0,
				m_maxSize
			);
			if (not m_view)
			{
				auto lastError = Win32::GetLastError();
				Close();
				throw Error::Win32Error(lastError, "MapViewOfFile() failed");
			}
		}

		std::wstring m_name;
		unsigned m_maxSize = 0;
		RAII::Win32Handle m_mapFile;
		void* m_view = nullptr;
	};
}