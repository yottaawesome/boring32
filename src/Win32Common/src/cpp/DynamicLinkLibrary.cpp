#include "../h/DynamicLinkLibrary.h"

namespace Win32Common
{
	DynamicLinkLibrary::DynamicLinkLibrary(wstring& path)
		: path(path)
	{
		std::unique_ptr<const char[]> ptr(GenericStrings::ConvertWCharToChar(path.c_str(), path.length()+1));
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
	
	DynamicLinkLibrary::~DynamicLinkLibrary()
	{
		FreeLibrary(libraryHandle);
	}
}
