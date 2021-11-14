module;

#include <Windows.h>
#include <string>
#include <stdexcept>

export module boring32.process.dynamiclinklibrary;

export namespace Boring32::Process
{
	class DynamicLinkLibrary
	{
		public:
			virtual ~DynamicLinkLibrary();
			DynamicLinkLibrary();
			DynamicLinkLibrary(const std::wstring& path);
			DynamicLinkLibrary(const std::wstring& path, const std::nothrow_t&) noexcept;

			DynamicLinkLibrary(const DynamicLinkLibrary& other);
			virtual DynamicLinkLibrary& operator=(const DynamicLinkLibrary& other);

			DynamicLinkLibrary(DynamicLinkLibrary&& other) noexcept;
			virtual DynamicLinkLibrary& operator=(DynamicLinkLibrary&& other) noexcept;

		public:
			virtual void Close() noexcept;
			virtual void* Resolve(const std::string& symbolName);
			virtual void* Resolve(const std::string& symbolName, const std::nothrow_t&) noexcept;
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
