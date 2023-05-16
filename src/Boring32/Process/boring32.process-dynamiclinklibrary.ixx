module;

#include <source_location>;

export module boring32.process:dynamiclinklibrary;
import <string>;
import <stdexcept>;
import <win32.hpp>;
import <format>;
import <iostream>;
import boring32.error;

export namespace Boring32::Process
{
	class DynamicLinkLibrary
	{
		public:
			virtual ~DynamicLinkLibrary()
			{
				Close();
			}

			DynamicLinkLibrary() = default;

			DynamicLinkLibrary(const DynamicLinkLibrary& other)
				: m_libraryHandle(nullptr)
			{
				Copy(other);
			}

			DynamicLinkLibrary(DynamicLinkLibrary&& other) noexcept
				: m_libraryHandle(nullptr)
			{
				Move(other);
			}

			DynamicLinkLibrary(const std::wstring& path)
				: m_path(path)
			{
				InternalLoad();
			}

			DynamicLinkLibrary(const std::wstring& path, const std::nothrow_t&) noexcept
				: m_path(path)
			{
				InternalLoad(std::nothrow);
			}

		public:
			virtual DynamicLinkLibrary& operator=(const DynamicLinkLibrary& other)
			{
				return Copy(other);
			}

			virtual DynamicLinkLibrary& operator=(DynamicLinkLibrary&& other) noexcept
			{
				return Move(other);
			}

		public:
			virtual void Close() noexcept
			{
				if (!m_libraryHandle)
					return;
				if (!FreeLibrary(m_libraryHandle))
				{
					const auto lastError = GetLastError();
					std::wcerr << std::format(L"FreeLibrary() failed: {}", lastError);
				}
				m_libraryHandle = nullptr;
				m_path.clear();
			}

			virtual void* Resolve(const std::string& symbolName)
			{
				if (!m_libraryHandle)
					throw Error::Boring32Error("Library handle is null");

				if (void* ptr = GetProcAddress(m_libraryHandle, symbolName.c_str()))
					return ptr;

				throw Error::Win32Error(
					std::format("{}: failed to resolve symbol: {}", __FUNCSIG__, symbolName),
					GetLastError());
			}

			virtual void* Resolve(
				const std::string& symbolName, 
				const std::nothrow_t&
			) noexcept
			{
				if (!m_libraryHandle)
					return nullptr;
				return GetProcAddress(m_libraryHandle, symbolName.c_str());
			}

			virtual const std::wstring& GetPath() const noexcept final
			{
				return m_path;
			}

			virtual HMODULE GetHandle() const noexcept final
			{
				return m_libraryHandle;
			}

			virtual bool IsLoaded() const noexcept final
			{
				return m_libraryHandle != nullptr;
			}
		
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
			virtual DynamicLinkLibrary& Copy(const DynamicLinkLibrary& other)
			{
				Close();
				m_path = other.m_path;
				InternalLoad();
				return *this;
			}

			virtual DynamicLinkLibrary& Move(DynamicLinkLibrary& other) noexcept
			{
				Close();
				m_path = std::move(other.m_path);
				m_libraryHandle = other.m_libraryHandle;
				other.m_libraryHandle = nullptr;
				return *this;
			}

			virtual void InternalLoad()
			{
				if (m_path.empty())
					throw Error::Boring32Error("No library path specified");

				m_libraryHandle = LoadLibraryW(m_path.c_str());
				if (!m_libraryHandle)
					throw Error::Win32Error("failed to load library", GetLastError());
			}

			virtual bool InternalLoad(
				const std::nothrow_t&
			) noexcept try
			{
				InternalLoad();
				return true;
			}
			catch (const std::exception& ex)
			{
				std::wcerr << ex.what() << std::endl;
				return false;
			}

		protected:
			std::wstring m_path;
			HMODULE m_libraryHandle = nullptr;
	};
}
