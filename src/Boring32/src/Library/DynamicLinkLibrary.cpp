#include "pch.hpp"
#include <stdexcept>
#include <sstream>
#include <iostream>
#include "include/Library/Library.hpp"
#include "include/Strings/Strings.hpp"
#include "include/Error/Error.hpp"

namespace Boring32::Library
{
	DynamicLinkLibrary::~DynamicLinkLibrary()
	{
		Close();
	}

	void DynamicLinkLibrary::Close() noexcept
	{
		if (m_libraryHandle != nullptr)
		{
			if (FreeLibrary(m_libraryHandle) == false)
				std::wcerr << __FUNCSIG__ << ": FreeLibrary() failed" << std::endl;
			m_libraryHandle = nullptr;
			m_path.clear();
		}
	}

	DynamicLinkLibrary::DynamicLinkLibrary()
		: m_libraryHandle(nullptr)
	{ }

	DynamicLinkLibrary::DynamicLinkLibrary(const std::wstring& path)
		: m_path(path)
	{
		InternalLoad();
	}

	DynamicLinkLibrary::DynamicLinkLibrary(const std::wstring& path, const std::nothrow_t& noThrow) noexcept
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

	void* DynamicLinkLibrary::Resolve(const std::wstring& symbolName)
	{
		if (m_libraryHandle == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": library handle is null");
		void* ptr = GetProcAddress(m_libraryHandle, Strings::ConvertWStringToString(m_path.c_str()).c_str());
		if (ptr == nullptr)
		{
			std::wstringstream wss;
			wss 
				<< __FUNCSIG__ ": failed to resolve symbol " 
				<< symbolName.c_str()
				<< std::endl;
			throw std::runtime_error(Strings::ConvertWStringToString(wss.str()));
		}
		return ptr;
	}
	
	void* DynamicLinkLibrary::Resolve(const std::wstring& symbolName, const std::nothrow_t& noThrow) noexcept
	{
		void* out = nullptr;
		Error::TryCatchLogToWCerr([this, &symbolName, out = &out] { *out = Resolve(symbolName); }, __FUNCSIG__);
		return out;
	}

	void DynamicLinkLibrary::InternalLoad()
	{
		if (m_path.empty())
			throw std::runtime_error(__FUNCSIG__ ": no library path specified");

		m_libraryHandle = LoadLibraryW(m_path.c_str());
		if (m_libraryHandle == nullptr)
			throw Error::Win32Error(__FUNCSIG__ ": failed to load library", GetLastError());
	}

	bool DynamicLinkLibrary::InternalLoad(const std::nothrow_t&) noexcept
	{
		return Error::TryCatchLogToWCerr([this]{ InternalLoad(); }, __FUNCSIG__);
	}
}
