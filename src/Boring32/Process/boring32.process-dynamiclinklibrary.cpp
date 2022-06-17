module;

#include <format>
#include <iostream>
#include <source_location>
#include <Windows.h>

module boring32.process:dynamiclinklibrary;
import boring32.error;

namespace Boring32::Process
{
	DynamicLinkLibrary::~DynamicLinkLibrary()
	{
		Close();
	}

	void DynamicLinkLibrary::Close() noexcept
	{
		if (!m_libraryHandle)
			return;
		if (!FreeLibrary(m_libraryHandle))
			std::wcerr << std::format(L"{}: FreeLibrary() failed: {}", TEXT(__FUNCSIG__), GetLastError());
		m_libraryHandle = nullptr;
		m_path.clear();
	}

	DynamicLinkLibrary::DynamicLinkLibrary()
		: m_libraryHandle(nullptr)
	{ }

	DynamicLinkLibrary::DynamicLinkLibrary(const std::wstring& path)
		: m_path(path)
	{
		InternalLoad();
	}

	DynamicLinkLibrary::DynamicLinkLibrary(const std::wstring& path, const std::nothrow_t&) noexcept
		: m_path(path)
	{
		InternalLoad(std::nothrow);
	}

	DynamicLinkLibrary::DynamicLinkLibrary(const DynamicLinkLibrary& other)
		: m_libraryHandle(nullptr)
	{
		Copy(other);
	}

	DynamicLinkLibrary::DynamicLinkLibrary(DynamicLinkLibrary&& other) noexcept
		: m_libraryHandle(nullptr)
	{
		Move(other);
	}

	DynamicLinkLibrary& DynamicLinkLibrary::operator=(DynamicLinkLibrary&& other) noexcept
	{
		return Move(other);
	}

	DynamicLinkLibrary& DynamicLinkLibrary::Move(DynamicLinkLibrary& other) noexcept
	{
		Close();
		m_path = std::move(other.m_path);
		m_libraryHandle = other.m_libraryHandle;
		other.m_libraryHandle = nullptr;
		return *this;
	}

	DynamicLinkLibrary& DynamicLinkLibrary::operator=(const DynamicLinkLibrary& other)
	{
		return Copy(other);
	}

	DynamicLinkLibrary& DynamicLinkLibrary::Copy(const DynamicLinkLibrary& other)
	{
		Close();
		m_path = other.m_path;
		InternalLoad();
		return *this;
	}

	const std::wstring& DynamicLinkLibrary::GetPath() const noexcept
	{
		return m_path;
	}

	HMODULE DynamicLinkLibrary::GetHandle() const noexcept
	{
		return m_libraryHandle;
	}
	
	bool DynamicLinkLibrary::IsLoaded() const noexcept
	{
		return m_libraryHandle != nullptr;
	}

	void* DynamicLinkLibrary::Resolve(const std::string& symbolName)
	{
		if (m_libraryHandle == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": library handle is null");
		
		if (void* ptr = GetProcAddress(m_libraryHandle, symbolName.c_str()))
			return ptr;

		throw Error::Win32Error(
			std::format("{}: failed to resolve symbol: {}", __FUNCSIG__, symbolName), 
			GetLastError());
	}
	
	void* DynamicLinkLibrary::Resolve(const std::string& symbolName, const std::nothrow_t&) noexcept
	{
		if (m_libraryHandle == nullptr)
			return nullptr;
		return GetProcAddress(m_libraryHandle, symbolName.c_str());
	}

	void DynamicLinkLibrary::InternalLoad()
	{
		if (m_path.empty())
			throw std::runtime_error(__FUNCSIG__ ": no library path specified");

		m_libraryHandle = LoadLibraryW(m_path.c_str());
		if (m_libraryHandle == nullptr)
			throw Error::Win32Error("failed to load library", GetLastError());
	}

	bool DynamicLinkLibrary::InternalLoad(const std::nothrow_t&) noexcept try
	{
		InternalLoad();
		return true;
	}
	catch (const std::exception& ex)
	{
		std::wcerr << ex.what() << std::endl;
		return false;
	}
}
