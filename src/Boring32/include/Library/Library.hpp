#pragma once
#include <Windows.h>
#include <string>
#include <stdexcept>

namespace Boring32::Library
{
	class DynamicLinkLibrary
	{
		public:
			DynamicLinkLibrary(const std::wstring& path);
			DynamicLinkLibrary(const std::wstring& path, const std::nothrow_t& noThrow);
			virtual ~DynamicLinkLibrary();
			virtual void* Resolve(const std::wstring& symbolName);
			virtual std::wstring GetPath() const;
			virtual HMODULE GetHandle() const;
			virtual bool IsLoaded() const;

		protected:
			const std::wstring m_path;
			HMODULE m_libraryHandle;
	};
}