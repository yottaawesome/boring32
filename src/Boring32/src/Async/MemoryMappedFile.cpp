#include "pch.hpp"
#include <stdexcept>
#include "include/Async/MemoryMappedFile.hpp"

namespace Boring32::Async
{
	MemoryMappedFile::MemoryMappedFile()
	:	m_mmfName(L""),
		m_maxSize(0),
		m_mapFile(nullptr),
		m_inheritable(false),
		m_view(nullptr)
	{ }
	
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
		m_view(nullptr)
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

		m_view = MapViewOfFile(
			m_mapFile,   // handle to map object
			FILE_MAP_ALL_ACCESS, // read/write permission
			0,
			0,
			maxSize
		);
		if (m_view == nullptr)
		{
			Cleanup();
			throw std::runtime_error("MapViewOfFile() failed");
		}

		// Don't zero the memory if we're opening an existing handle, as it may have data.
		if (createFile)
		{
			SecureZeroMemory(m_view, m_maxSize);
		}
	}

	MemoryMappedFile::MemoryMappedFile(const MemoryMappedFile& other)
	:	m_mmfName(other.m_mmfName),
		m_maxSize(other.m_maxSize),
		m_mapFile(nullptr),
		m_inheritable(other.m_inheritable)
	{
		Duplicate(other);
	}
	
	void MemoryMappedFile::operator=(const MemoryMappedFile& other)
	{
		Cleanup();
		Duplicate(other);
	}

	void MemoryMappedFile::Duplicate(const MemoryMappedFile& other)
	{
		m_mmfName = other.m_mmfName;
		m_maxSize = other.m_maxSize;
		m_mapFile = nullptr;
		m_inheritable = other.m_inheritable;

		if (other.m_mapFile != nullptr)
		{
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

			m_view = MapViewOfFile(
				m_mapFile,   // handle to map object
				FILE_MAP_ALL_ACCESS, // read/write permission
				0,
				0,
				m_maxSize
			);
			if (m_view == nullptr)
			{
				Cleanup();
				throw std::runtime_error("MapViewOfFile() failed");
			}
		}
	}

	MemoryMappedFile::MemoryMappedFile(MemoryMappedFile&& other) noexcept
	{
		Move(other);
	}

	void MemoryMappedFile::operator=(MemoryMappedFile&& other) noexcept
	{
		Cleanup();
		Move(other);
	}

	void MemoryMappedFile::Move(MemoryMappedFile& other) noexcept
	{
		m_mmfName = other.m_mmfName;
		m_maxSize = other.m_maxSize;
		m_mapFile = other.m_mapFile;
		m_view = other.m_view;
		m_inheritable = other.m_inheritable;
		other.m_mapFile = nullptr;
		other.m_view = nullptr;
	}

	void* MemoryMappedFile::GetViewPointer()
	{
		return m_view;
	}

	void MemoryMappedFile::Cleanup()
	{
		if (m_view)
		{
			UnmapViewOfFile(m_view);
			m_view = nullptr;
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
