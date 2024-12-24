export module boring32:async_filemapping;
import boring32.shared;
import boring32.win32;
import :raii;
import :error;

export namespace Boring32::Async
{
	struct FileMapping final
	{
		~FileMapping()
		{
			Close();
		}

		FileMapping() = default;

		FileMapping(
			const bool isInheritable, 
			const size_t maxSize, 
			const Win32::DWORD pageProtection
		) : m_maxSize(maxSize),
			m_pageProtection(pageProtection)
		{
			Create(Win32::FileMapAccess::All, isInheritable);
		}

		/// Create a new page-backed file mapping.
		FileMapping(
			const bool isInheritable, 
			const std::wstring name, 
			const size_t maxSize, 
			const Win32::DWORD pageProtection
		) : m_name(std::move(name)),
			m_maxSize(maxSize),
			m_pageProtection(pageProtection)
		{
			if (m_name.empty())
				throw Error::Boring32Error("Name cannot be an empty string");
			if (m_maxSize == 0)
				throw Error::Boring32Error("maxSize cannot be 0");
			if (m_pageProtection == 0)
				throw Error::Boring32Error("pageProtection cannot be 0");
			Create(Win32::FileMapAccess::All, isInheritable);
		}

		/// Open an existing named page file with specific access.
		FileMapping(
			const std::wstring name,
			const Win32::FileMapAccess desiredAccess,
			const bool isInheritable,
			const size_t maxSize
		) : m_name(std::move(name)),
			m_maxSize(maxSize),
			m_pageProtection(0)
		{
			if (m_name.empty())
				throw Error::Boring32Error("name cannot be an empty string");
			if (m_maxSize == 0)
				throw Error::Boring32Error("maxSize cannot be 0");
			if (m_pageProtection == 0)
				throw Error::Boring32Error("pageProtection cannot be 0");
			Open(desiredAccess, isInheritable);
		}

		FileMapping(const FileMapping& other)
			: FileMapping() 
		{
			Copy(other);
		}

		FileMapping(FileMapping&& other) noexcept
			: FileMapping() 
		{
			Move(other);
		}

		FileMapping& operator=(const FileMapping& other)
		{
			return Copy(other);
		}

		FileMapping& operator=(FileMapping&& other) noexcept
		{
			return Move(other);
		}

		void Close()
		{
			m_fileMapping = nullptr;
			m_maxSize = 0;
			m_name.clear();
			m_pageProtection = 0;
		}

		const std::wstring& GetName() const noexcept
		{
			return m_name;
		}

		Win32::HANDLE GetNativeHandle() const noexcept
		{
			return m_fileMapping.GetHandle();
		}

		RAII::Win32Handle GetHandle() const noexcept
		{
			return m_fileMapping;
		}

		size_t GetFileSize() const
		{
			return m_maxSize;
		}
			
		private:
		void Create(const Win32::FileMapAccess desiredAccess, const bool isInheritable)
		{
			Win32::LARGE_INTEGER li{ .QuadPart = static_cast<long long>(m_maxSize) };
			const wchar_t* name = m_name.empty() ? nullptr : m_name.c_str();
			// https://docs.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-createfilemappingw
			m_fileMapping = Win32::CreateFileMappingW(
				Win32::InvalidHandleValue,	// use paging file
				nullptr,				// default security
				m_pageProtection,		// read/write access. e.g. PAGE_READWRITE
				li.HighPart,			// maximum object size (high-order DWORD)
				li.LowPart,				// maximum object size (low-order DWORD)
				name					// m_name of mapping object
			);
			if (not m_fileMapping)
				throw Error::Win32Error(Win32::GetLastError(), "CreateFileMappingW() failed");

			m_fileMapping.SetInheritability(isInheritable);
		}

		void Open(const Win32::FileMapAccess desiredAccess, const bool isInheritable)
		{
			if (m_name.empty())
				throw Error::Boring32Error("m_name cannot be empty when opening a file mapping");

			// https://docs.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-openfilemappingw
			m_fileMapping = Win32::OpenFileMappingW(
				static_cast<Win32::DWORD>(desiredAccess),	// read/write access
				isInheritable,						// Should the handle be inheritable
				m_name.c_str()						// name of mapping object
			);
			if (not m_fileMapping)
				throw Error::Win32Error(Win32::GetLastError(), "OpenFileMappingW() failed");
		}

		FileMapping& Copy(const FileMapping& other)
		{
			Close();
			m_name = other.m_name;
			m_maxSize = other.m_maxSize;
			m_pageProtection = other.m_pageProtection;
			// This...?
			m_fileMapping = other.m_fileMapping;
			// Or this...?
			/*if (other.m_fileMapping)
				CreateOrOpen(false, FileMapAccess::All, other.m_fileMapping.IsInheritable());*/
				// Or perhaps we shouldn't allow sharing? The problem with sharing is if the 
				// object stores some aspect of the handle but the handle is changed via another
				// object, then the first object doesn't know about it.
			return *this;
		}

		FileMapping& Move(FileMapping& other) noexcept
		{
			Close();
			m_fileMapping = std::move(other.m_fileMapping);
			m_name = std::move(other.m_name);
			m_maxSize = other.m_maxSize;
			m_pageProtection = other.m_pageProtection;
			other.Close();
			return *this;
		}

		RAII::Win32Handle m_fileMapping;
		size_t m_maxSize = 0;
		std::wstring m_name;
		Win32::DWORD m_pageProtection = 0;
	};
}
