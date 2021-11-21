#include "pch.hpp"
#include <stdexcept>
#include <format>
#include "include/Async/MemoryMappedFile.hpp"

import boring32.error.win32error;

namespace Boring32::Async
{
	void MemoryMappedFile::Close()
	{
		if (m_view && !UnmapViewOfFile(m_view))
			throw Error::Win32Error(__FUNCSIG__": UnmapViewOfFile() failed");
		m_view = nullptr;
		m_mapFile = nullptr;
	}

	void MemoryMappedFile::Close(const std::nothrow_t&) try
	{
		Close();
	}
	catch(const std::exception& ex)
	{
		std::wcerr << std::format(__FUNCSIG__": Close() failed: {}\n", ex.what()).c_str();
	}

	MemoryMappedFile::~MemoryMappedFile()
	{
		Close(std::nothrow);
	}

	MemoryMappedFile::MemoryMappedFile()
	:	m_name(L""),
		m_maxSize(0),
		m_mapFile(nullptr),
		m_view(nullptr)
	{ }
	
	MemoryMappedFile::MemoryMappedFile(
		std::wstring name,
		const UINT maxSize,
		const bool inheritable
	)
	:	m_name(std::move(name)),
		m_maxSize(maxSize),
		m_mapFile(nullptr),
		m_view(nullptr)
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
			throw Error::Win32Error(__FUNCSIG__": CreateFileMappingW() failed", GetLastError());

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
			throw Error::Win32Error(__FUNCSIG__": MapViewOfFile() failed", GetLastError());
		}

		RtlSecureZeroMemory(m_view, m_maxSize);
	}

	MemoryMappedFile::MemoryMappedFile(
		std::wstring name,
		const UINT maxSize,
		const bool inheritable,
		const DWORD desiredAccess
	)
	:	m_name(std::move(name)),
		m_maxSize(maxSize),
		m_mapFile(nullptr),
		m_view(nullptr)
	{
		// desiredAccess: https://docs.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-mapviewoffile
		m_mapFile = OpenFileMappingW(
			desiredAccess,	// read/write access
			inheritable,	// Should the handle be inheritable
			m_name.c_str()	// name of mapping object
		);
		if (!m_mapFile)
			throw Error::Win32Error(__FUNCSIG__": OpenFileMappingW() failed", GetLastError());

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
			throw Error::Win32Error(__FUNCSIG__": MapViewOfFile() failed", GetLastError());
		}
	}

	MemoryMappedFile::MemoryMappedFile(const MemoryMappedFile& other)
	:	m_name(other.m_name),
		m_maxSize(other.m_maxSize),
		m_mapFile(nullptr)
	{
		Copy(other);
	}
	
	MemoryMappedFile& MemoryMappedFile::operator=(const MemoryMappedFile& other)
	{
		Close();
		Copy(other);
		return *this;
	}

	void MemoryMappedFile::Copy(const MemoryMappedFile& other)
	{
		Close();
		m_name = other.m_name;
		m_maxSize = other.m_maxSize;
		m_mapFile = other.m_mapFile;
		if (m_mapFile)
		{
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
				throw Error::Win32Error(__FUNCSIG__": MapViewOfFile() failed", GetLastError());
			}
		}
	}

	MemoryMappedFile::MemoryMappedFile(MemoryMappedFile&& other) noexcept
	{
		Move(other);
	}

	MemoryMappedFile& MemoryMappedFile::operator=(MemoryMappedFile&& other) noexcept
	{
		Move(other);
		return *this;
	}

	void MemoryMappedFile::Move(MemoryMappedFile& other) noexcept
	{
		Close();
		m_name = std::move(other.m_name);
		m_mapFile = std::move(other.m_mapFile);
		m_maxSize = other.m_maxSize;
		m_view = other.m_view;
		other.m_view = nullptr;
	}

	void* MemoryMappedFile::GetViewPointer() const noexcept
	{
		return m_view;
	}

	const std::wstring& MemoryMappedFile::GetName() const noexcept
	{
		return m_name;
	}

	bool MemoryMappedFile::IsInheritable() const
	{
		return m_mapFile.IsInheritable();
	}
}
