#pragma once
#include <Windows.h>
#include <string>

namespace Win32Utils::Process
{
	class DynamicLinkLibrary
	{
		public:
			DynamicLinkLibrary(const std::wstring& path);
			virtual ~DynamicLinkLibrary();
			virtual const std::wstring& GetPath();
			virtual const HMODULE GetHandle();
			virtual void* Resolve(const std::wstring& path);

		protected:
			const std::wstring path;
			HMODULE libraryHandle;
	};
}