#include "pch.hpp"
#include <stdexcept>
#include "include/Boring32.hpp"

namespace Boring32::Library
{
	DynamicLinkLibrary::DynamicLinkLibrary(const std::wstring& path)
		: m_path(path)
	{
		m_libraryHandle = LoadLibrary(path.c_str());
		if (m_libraryHandle == nullptr)
			throw std::runtime_error("Failed to load library");
	}

	DynamicLinkLibrary::DynamicLinkLibrary(const std::wstring& path, const std::nothrow_t& noThrow)
	{
		m_libraryHandle = LoadLibrary(path.c_str());
	}

	std::wstring DynamicLinkLibrary::GetPath() const
	{
		return m_path;
	}

	HMODULE DynamicLinkLibrary::GetHandle() const
	{
		return m_libraryHandle;
	}
	
	bool DynamicLinkLibrary::IsLoaded() const
	{
		return m_libraryHandle != nullptr;
	}

	void* DynamicLinkLibrary::Resolve(const std::wstring& symbolName)
	{
		if (m_libraryHandle == nullptr)
			throw std::runtime_error("Library handle is null");
		return GetProcAddress(m_libraryHandle, Strings::ConvertWStringToString(m_path.c_str()).c_str());
	}

	DynamicLinkLibrary::~DynamicLinkLibrary()
	{
		if (m_libraryHandle != nullptr)
		{
			FreeLibrary(m_libraryHandle);
			m_libraryHandle = nullptr;
		}
	}
}
