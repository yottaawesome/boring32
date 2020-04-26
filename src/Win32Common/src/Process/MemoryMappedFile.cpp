#include "pch.hpp"
#include "include/Win32Utils.hpp"
#include <stdexcept>

namespace Win32Utils::Process
{
	MemoryMappedFile::MemoryMappedFile(
		const std::wstring& name,
		const UINT maxSize,
		const bool createFile,
		const bool inheritable
	)
	:	m_mmfName(name),
		m_maxSize(maxSize),
		m_MapFile(nullptr),
		m_createFile(createFile),
		m_initialised(false),
		m_inheritable(inheritable)
	{
		if (createFile)
		{
			SECURITY_ATTRIBUTES lp{ 0 };
			lp.nLength = sizeof(lp);
			lp.bInheritHandle = m_inheritable;
			m_MapFile = CreateFileMapping(
				INVALID_HANDLE_VALUE,    // use paging file
				&lp,                    // default security
				PAGE_READWRITE,          // read/write access
				0,                       // maximum object size (high-order DWORD)
				m_maxSize,                // maximum object size (low-order DWORD)
				m_mmfName.c_str());                 // m_name of mapping object
		}
		else
		{
			m_MapFile = OpenFileMapping(
				FILE_MAP_ALL_ACCESS,   // read/write access
				m_inheritable,        // Should the handle be inheritable
				m_mmfName.c_str()	// name of mapping object
			);
		}

		if (m_MapFile == nullptr)
		{
			Cleanup();
			throw std::runtime_error("Failed to open memory mapped file");
		}

		m_View = (void*)MapViewOfFile(
			m_MapFile,   // handle to map object
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

		m_initialised = true;
	}

	void* MemoryMappedFile::GetViewPointer()
	{
		return m_View;
	}

	bool MemoryMappedFile::Initialised()
	{
		return m_initialised;
	}

	void MemoryMappedFile::Cleanup()
	{
		if (m_View)
		{
			UnmapViewOfFile(m_View);
			m_View = nullptr;
		}
		if (m_MapFile)
		{
			CloseHandle(m_MapFile);
			m_MapFile = nullptr;
		}
	}

	/*
	void MemoryMappedFile::operator=(const MemoryMappedFile& other)
	{
		if (other.m_initialised)
			;
	}
	*/

	MemoryMappedFile::~MemoryMappedFile()
	{
		Cleanup();
	}
}
