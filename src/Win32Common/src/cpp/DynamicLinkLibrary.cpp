#include "../../include/Win32Common.h"
#include <stdexcept>

namespace Win32Common
{
	DynamicLinkLibrary::DynamicLinkLibrary(const std::wstring& path)
		: path(path)
	{
		libraryHandle = LoadLibrary(path.c_str());
		if (libraryHandle == nullptr)
			throw std::runtime_error("Failed to load library");
	}

	const std::wstring& DynamicLinkLibrary::GetPath()
	{
		return path;
	}

	const HMODULE DynamicLinkLibrary::GetHandle()
	{
		return this->libraryHandle;
	}

	void* DynamicLinkLibrary::Resolve(const std::wstring& path)
	{
		return GetProcAddress(this->libraryHandle, Win32Strings::ConvertWStringToString(path.c_str()).c_str());
	}

	DynamicLinkLibrary::~DynamicLinkLibrary()
	{
		FreeLibrary(libraryHandle);
	}
}
