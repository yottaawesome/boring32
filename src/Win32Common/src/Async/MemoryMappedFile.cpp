#include "pch.hpp"
#include "include/Win32Utils.hpp"
#include <stdexcept>

namespace Win32Utils::Async
{
	MemoryMappedFile::MemoryMappedFile(
		const std::wstring& name,
		const UINT maxSize,
		const bool createFile,
		const bool inheritable
	)
	:	m_mmfName(name),
		m_maxSize(maxSize),
		m_mapFile(nullptr),
		m_inheritable(inheritable),
		m_View(nullptr)
	{
		if (createFile)
		{
			SECURITY_ATTRIBUTES lp{ 0 };
			lp.nLength = sizeof(lp);
			lp.bInheritHandle = m_inheritable;
			m_mapFile = CreateFileMapping(
				INVALID_HANDLE_VALUE,    // use paging file
				&lp,                    // default security
				PAGE_READWRITE,          // read/write access
				0,                       // maximum object size (high-order DWORD)
				m_maxSize,                // maximum object size (low-order DWORD)
				m_mmfName.c_str());                 // m_name of mapping object
		}
		else
		{
			m_mapFile = OpenFileMapping(
				FILE_MAP_ALL_ACCESS,   // read/write access
				m_inheritable,        // Should the handle be inheritable
				m_mmfName.c_str()	// name of mapping object
			);
		}

		if (m_mapFile == nullptr)
		{
			Cleanup();
			throw std::runtime_error("Failed to open memory mapped file");
		}

		m_View = MapViewOfFile(
			m_mapFile,   // handle to map object
			FILE_MAP_ALL_ACCESS, // read/write permission
			0,
			0,
			maxSize
		);
		if (m_View == nullptr)
		{
			Cleanup();
			throw std::runtime_error("MapViewOfFile() failed");
		}

		// Don't zero the memory if we're opening an existing handle, as it may have data.
		if (createFile)
		{
			SecureZeroMemory(m_View, m_maxSize);
		}
	}

	MemoryMappedFile::MemoryMappedFile(const MemoryMappedFile& other)
	:	m_mmfName(other.m_mmfName),
		m_maxSize(other.m_maxSize),
		m_mapFile(nullptr),
		m_inheritable(other.m_inheritable)
	{
		if (other.m_mapFile == nullptr)
			throw std::runtime_error("Invalid state");

		Duplicate(other);
	}
	
	void MemoryMappedFile::operator=(const MemoryMappedFile& other)
	{
		if (other.m_mapFile == nullptr)
			throw std::runtime_error("Invalid state");
		Cleanup();

		Duplicate(other);
	}

	void MemoryMappedFile::Duplicate(const MemoryMappedFile& other)
	{
		m_mmfName = other.m_mmfName;
		m_maxSize = other.m_maxSize;
		m_mapFile = nullptr;
		m_inheritable = other.m_inheritable;

		bool succeeded = DuplicateHandle(
			GetCurrentProcess(),
			other.m_mapFile,
			GetCurrentProcess(),
			&m_mapFile,
			0,
			m_inheritable,
			DUPLICATE_SAME_ACCESS
		);
		if (succeeded == false)
			throw std::runtime_error("Failed to duplicated handle.");

		m_View = MapViewOfFile(
			m_mapFile,   // handle to map object
			FILE_MAP_ALL_ACCESS, // read/write permission
			0,
			0,
			m_maxSize
		);
		if (m_View == nullptr)
		{
			Cleanup();
			throw std::runtime_error("MapViewOfFile() failed");
		}
	}

	void* MemoryMappedFile::GetViewPointer()
	{
		return m_View;
	}

	void MemoryMappedFile::Cleanup()
	{
		if (m_View)
		{
			UnmapViewOfFile(m_View);
			m_View = nullptr;
		}
		if (m_mapFile)
		{
			CloseHandle(m_mapFile);
			m_mapFile = nullptr;
		}
	}

	MemoryMappedFile::~MemoryMappedFile()
	{
		Cleanup();
	}
}
