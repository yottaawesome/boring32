module;

#include <source_location>

module boring32.async:filemapping;
import boring32.error;
import <stdexcept>;

namespace Boring32::Async
{
	FileMapping::~FileMapping()
	{
		Close();
	}

	FileMapping::FileMapping()
		: m_maxSize(0),
		m_pageProtection(0)
	{}

	FileMapping::FileMapping(const bool isInheritable, const size_t maxSize, const DWORD pageProtection)
		: m_maxSize(maxSize),
		m_pageProtection(pageProtection)
	{
		CreateOrOpen(true, FileMapAccess::All, isInheritable);
	}
	
	FileMapping::FileMapping(
		const bool isInheritable, 
		const std::wstring name, 
		const size_t maxSize,
		const DWORD pageProtection
	)
	:	m_name(std::move(name)), 
		m_maxSize(maxSize),
		m_pageProtection(pageProtection)
	{
		if (m_name.empty())
			throw Error::Boring32Error("Name cannot be an empty string");
		if (m_maxSize == 0)
			throw Error::Boring32Error("maxSize cannot be 0");
		if (m_pageProtection == 0)
			throw Error::Boring32Error("pageProtection cannot be 0");
		CreateOrOpen(true, FileMapAccess::All, isInheritable);
	}

	FileMapping::FileMapping(const FileMapping& other) : FileMapping() { Copy(other); }
	FileMapping::FileMapping(FileMapping&& other) noexcept : FileMapping() { Move(other); }

	FileMapping::FileMapping(
		const std::wstring name,
		const FileMapAccess desiredAccess,
		const bool isInheritable,
		const size_t maxSize
	)
	:	m_name(std::move(name)),
		m_maxSize(maxSize),
		m_pageProtection(0)
	{
		if (m_name.empty())
			throw Error::Boring32Error("name cannot be an empty string");
		if (m_maxSize == 0)
			throw Error::Boring32Error("maxSize cannot be 0");
		if (m_pageProtection == 0)
			throw Error::Boring32Error("pageProtection cannot be 0");
		CreateOrOpen(false, desiredAccess, isInheritable);
	}

	FileMapping& FileMapping::operator=(const FileMapping& other) { return Copy(other); }
	FileMapping& FileMapping::operator=(FileMapping&& other) noexcept { return Move(other); }

	void FileMapping::Close()
	{
		m_fileMapping = nullptr;
		m_maxSize = 0;
		m_name.clear();
		m_pageProtection = 0;
	}

	const std::wstring FileMapping::GetName() const noexcept
	{
		return m_name;
	}

	HANDLE FileMapping::GetNativeHandle() const noexcept
	{
		return m_fileMapping.GetHandle();
	}

	const RAII::Win32Handle FileMapping::GetHandle() const noexcept
	{
		return m_fileMapping;
	}
	
	size_t FileMapping::GetFileSize() const
	{
		return m_maxSize;
	}

	#define LODWORD(_qw)    ((DWORD)(_qw))
	#define HIDWORD(_qw)    ((DWORD)(((_qw) >> 32) & 0xffffffff))

	void FileMapping::CreateOrOpen(
		const bool create,
		const FileMapAccess desiredAccess,
		const bool isInheritable
	)
	{
		if (create)
		{
			LARGE_INTEGER li{ .QuadPart = static_cast<long long>(m_maxSize) };
			const wchar_t* name = m_name.empty() ? nullptr : m_name.c_str();
			// https://docs.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-createfilemappingw
			m_fileMapping = CreateFileMappingW(
				INVALID_HANDLE_VALUE,	// use paging file
				nullptr,				// default security
				m_pageProtection,		// read/write access. e.g. PAGE_READWRITE
				li.HighPart,			// maximum object size (high-order DWORD)
				li.LowPart,				// maximum object size (low-order DWORD)
				name					// m_name of mapping object
			);
			if (!m_fileMapping)
				throw Error::Win32Error("CreateFileMappingW() failed", GetLastError());
			m_fileMapping.SetInheritability(isInheritable);
		}
		else
		{
			if (m_name.empty())
				throw Error::Boring32Error("m_name cannot be empty when opening a file mapping");
			// https://docs.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-openfilemappingw
			m_fileMapping = OpenFileMappingW(
				static_cast<DWORD>(desiredAccess),	// read/write access
				isInheritable,						// Should the handle be inheritable
				m_name.c_str()						// name of mapping object
			);
			if (!m_fileMapping)
				throw Error::Win32Error("OpenFileMappingW() failed", GetLastError());
		}
	}

	FileMapping& FileMapping::Copy(const FileMapping& other)
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

	FileMapping& FileMapping::Move(FileMapping& other) noexcept
	{
		Close();
		m_fileMapping = std::move(other.m_fileMapping);
		m_name = std::move(other.m_name);
		m_maxSize = other.m_maxSize;
		m_pageProtection = other.m_pageProtection;
		other.Close();
		return *this;
	}
}