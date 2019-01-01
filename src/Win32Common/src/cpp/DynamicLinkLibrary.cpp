#include "../../include/Win32Common.h"

namespace Win32Common
{
	DynamicLinkLibrary::DynamicLinkLibrary(const wstring& path)
		: path(path)
	{
		libraryHandle = LoadLibrary(path.c_str());
		if (libraryHandle == nullptr)
			throw std::runtime_error("Failed to load library");
	}

	const wstring& DynamicLinkLibrary::GetPath()
	{
		return path;
	}

	const HMODULE DynamicLinkLibrary::GetHandle()
	{
		return this->libraryHandle;
	}

	void* DynamicLinkLibrary::Resolve(const wstring& path)
	{
		return GetProcAddress(this->libraryHandle, Win32Strings::ConvertWStringToString(path.c_str()).c_str());
	}

	DynamicLinkLibrary::~DynamicLinkLibrary()
	{
		FreeLibrary(libraryHandle);
	}
}
