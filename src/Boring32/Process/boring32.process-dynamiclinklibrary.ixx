module;

#include <Windows.h>
#include <string>
#include <stdexcept>

export module boring32.process:dynamiclinklibrary;

export namespace Boring32::Process
{
	class DynamicLinkLibrary
	{
		public:
			virtual ~DynamicLinkLibrary();
			DynamicLinkLibrary();
			DynamicLinkLibrary(const DynamicLinkLibrary& other);
			DynamicLinkLibrary(DynamicLinkLibrary&& other) noexcept;

			DynamicLinkLibrary(const std::wstring& path);
			DynamicLinkLibrary(const std::wstring& path, const std::nothrow_t&) noexcept;

		public:
			virtual DynamicLinkLibrary& operator=(const DynamicLinkLibrary& other);
			virtual DynamicLinkLibrary& operator=(DynamicLinkLibrary&& other) noexcept;

		public:
			virtual void Close() noexcept;
			virtual void* Resolve(const std::string& symbolName);
			virtual void* Resolve(const std::string& symbolName, const std::nothrow_t&) noexcept;
			virtual const std::wstring& GetPath() const noexcept final;
			virtual HMODULE GetHandle() const noexcept final;
			virtual bool IsLoaded() const noexcept final;
		
		public:
			template<typename T>
			T Resolve(const std::string& symbolName)
			{
				return static_cast<T>(Resolve(symbolName));
			}

			template<typename T>
			T Resolve(const std::string& symbolName, const std::nothrow_t&) noexcept
			{
				return static_cast<T>(Resolve(symbolName, std::nothrow));
			}

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
