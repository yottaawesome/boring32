#include "pch.hpp"
#include <stdexcept>
#include "include/Async/MemoryMappedFile.hpp"

namespace Boring32::Async
{
	void MemoryMappedFile::Close()
	{
		if (m_view)
		{
			UnmapViewOfFile(m_view);
			m_view = nullptr;
		}
		if (m_mapFile != nullptr)
		{
			m_mapFile.Close();
			m_mapFile = nullptr;
		}
	}

	MemoryMappedFile::~MemoryMappedFile()
	{
		Close();
	}

	MemoryMappedFile::MemoryMappedFile()
	:	m_name(L""),
		m_maxSize(0),
		m_mapFile(nullptr),
		m_view(nullptr)
	{ }
	
	MemoryMappedFile::MemoryMappedFile(
		const std::wstring& name,
		const UINT maxSize,
		const bool createFile,
		const bool inheritable
	)
	:	m_name(name),
		m_maxSize(maxSize),
		m_mapFile(nullptr),
		m_view(nullptr)
	{
		if (createFile)
		{
			SECURITY_ATTRIBUTES lp{ 0 };
			lp.nLength = sizeof(lp);
			lp.bInheritHandle = inheritable;
			m_mapFile = CreateFileMapping(
				INVALID_HANDLE_VALUE,		// use paging file
				&lp,						// default security
				PAGE_READWRITE,				// read/write access
				0,							// maximum object size (high-order DWORD)
				m_maxSize,					// maximum object size (low-order DWORD)
				m_name.c_str());			// m_name of mapping object
		}
		else
		{
			m_mapFile = OpenFileMapping(
				FILE_MAP_ALL_ACCESS,	// read/write access
				inheritable,			// Should the handle be inheritable
				m_name.c_str()		// name of mapping object
			);
		}
		if (m_mapFile == nullptr)
		{
			Close();
			throw std::runtime_error("Failed to open memory mapped file");
		}

		m_view = MapViewOfFile(
			m_mapFile.GetHandle(),	// handle to map object
			FILE_MAP_ALL_ACCESS,	// read/write permission
			0,
			0,
			maxSize
		);
		if (m_view == nullptr)
		{
			Close();
			throw std::runtime_error("MapViewOfFile() failed");
		}

		// Don't zero the memory if we're opening an existing handle, as it may have data.
		if (createFile)
			SecureZeroMemory(m_view, m_maxSize);
	}

	MemoryMappedFile::MemoryMappedFile(const MemoryMappedFile& other)
	:	m_name(other.m_name),
		m_maxSize(other.m_maxSize),
		m_mapFile(nullptr)
	{
		Copy(other);
	}
	
	void MemoryMappedFile::operator=(const MemoryMappedFile& other)
	{
		Close();
		Copy(other);
	}

	void MemoryMappedFile::Copy(const MemoryMappedFile& other)
	{
		Close();
		m_name = other.m_name;
		m_maxSize = other.m_maxSize;
		m_mapFile = other.m_mapFile;
		if (m_mapFile != nullptr)
		{
			m_view = MapViewOfFile(
				m_mapFile.GetHandle(),   // handle to map object
				FILE_MAP_ALL_ACCESS, // read/write permission
				0,
				0,
				m_maxSize
			);
			if (m_view == nullptr)
			{
				Close();
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
		Move(other);
	}

	void MemoryMappedFile::Move(MemoryMappedFile& other) noexcept
	{
		Close();
		m_name = std::move(other.m_name);
		m_mapFile = std::move(other.m_mapFile);
		m_maxSize = other.m_maxSize;
		m_view = other.m_view;
		other.m_mapFile = nullptr;
		other.m_view = nullptr;
	}

	void* MemoryMappedFile::GetViewPointer()
	{
		return m_view;
	}

	std::wstring MemoryMappedFile::GetName()
	{
		return m_name;
	}

	bool MemoryMappedFile::IsInheritable()
	{
		return m_mapFile.IsInheritable();
	}
}
