export module boring32.process:dynamiclinklibrary;
import std;

import <win32.hpp>;
import boring32.error;

export namespace Boring32::Process
{
	class DynamicLinkLibrary final
	{
		public:
			~DynamicLinkLibrary()
			{
				Close();
			}

			DynamicLinkLibrary() = default;

			DynamicLinkLibrary(const DynamicLinkLibrary& other)
			{
				Copy(other);
			}

			DynamicLinkLibrary(DynamicLinkLibrary&& other) noexcept
			{
				Move(other);
			}

			DynamicLinkLibrary& operator=(const DynamicLinkLibrary& other)
			{
				return Copy(other);
			}

			DynamicLinkLibrary& operator=(DynamicLinkLibrary&& other) noexcept
			{
				return Move(other);
			}

		public:
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
			void Close() noexcept
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

			void* Resolve(const std::string& symbolName)
			{
				if (!m_libraryHandle)
					throw Error::Boring32Error("Library handle is null");

				if (void* ptr = GetProcAddress(m_libraryHandle, symbolName.c_str()))
					return ptr;

				const auto lastError = GetLastError();
				throw Error::Win32Error(
					std::format("Failed to resolve symbol: {}", symbolName),
					lastError
				);
			}

			void* Resolve(
				const std::string& symbolName, 
				const std::nothrow_t&
			) noexcept
			{
				if (!m_libraryHandle)
					return nullptr;
				return GetProcAddress(m_libraryHandle, symbolName.c_str());
			}

			const std::wstring& GetPath() const noexcept
			{
				return m_path;
			}

			HMODULE GetHandle() const noexcept
			{
				return m_libraryHandle;
			}

			bool IsLoaded() const noexcept
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

		private:
			DynamicLinkLibrary& Copy(const DynamicLinkLibrary& other)
			{
				Close();
				m_path = other.m_path;
				InternalLoad();
				return *this;
			}

			DynamicLinkLibrary& Move(DynamicLinkLibrary& other) noexcept
			{
				Close();
				m_path = std::move(other.m_path);
				m_libraryHandle = other.m_libraryHandle;
				other.m_libraryHandle = nullptr;
				return *this;
			}

			void InternalLoad()
			{
				if (m_path.empty())
					throw Error::Boring32Error("No library path specified");

				m_libraryHandle = LoadLibraryW(m_path.c_str());
				if (!m_libraryHandle)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("failed to load library", lastError);
				}
			}

			bool InternalLoad(
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

		private:
			std::wstring m_path;
			HMODULE m_libraryHandle = nullptr;
	};
}
