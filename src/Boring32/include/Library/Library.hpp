#pragma once
#include <Windows.h>
#include <string>
#include <stdexcept>

namespace Boring32::Library
{
	class DynamicLinkLibrary
	{
		public:
			virtual ~DynamicLinkLibrary();
			DynamicLinkLibrary();
			DynamicLinkLibrary(const std::wstring& path);
			DynamicLinkLibrary(const std::wstring& path, const std::nothrow_t& noThrow) noexcept;

			DynamicLinkLibrary(const DynamicLinkLibrary& other);
			virtual DynamicLinkLibrary& operator=(const DynamicLinkLibrary& other);

			DynamicLinkLibrary(DynamicLinkLibrary&& other) noexcept;
			virtual DynamicLinkLibrary& operator=(DynamicLinkLibrary&& other) noexcept;

		public:
			virtual void Close() noexcept;
			virtual void* Resolve(const std::wstring& symbolName);
			virtual void* Resolve(const std::wstring& symbolName, const std::nothrow_t& noThrow) noexcept;
			virtual const std::wstring& GetPath() const noexcept;
			virtual HMODULE GetHandle() const noexcept;
			virtual bool IsLoaded() const noexcept;

		protected:
			virtual DynamicLinkLibrary& Copy(const DynamicLinkLibrary& other);
			virtual DynamicLinkLibrary& Move(DynamicLinkLibrary& other) noexcept;
			virtual void InternalLoad();
			virtual bool InternalLoad(const std::nothrow_t&) noexcept;

		protected:
			std::wstring m_path;
			HMODULE m_libraryHandle;
	};
}