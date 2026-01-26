export module boring32:process.dynamiclinklibrary;
import std;
import boring32.win32;
import :error;

export namespace Boring32::Process
{
	struct DynamicLinkLibrary final
	{
		~DynamicLinkLibrary()
		{
			Close();
		}

		constexpr DynamicLinkLibrary() = default;

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

		void Close() noexcept
		{
			if (not m_libraryHandle)
				return;
			if (not Win32::FreeLibrary(m_libraryHandle))
			{
				const auto lastError = Win32::GetLastError();
				std::wcerr << std::format(L"FreeLibrary() failed: {}", lastError);
			}
			m_libraryHandle = nullptr;
			m_path.clear();
		}

		void* Resolve(const std::string& symbolName)
		{
			if (not m_libraryHandle)
				throw Error::Boring32Error("Library handle is null");

			if (void* ptr = Win32::GetProcAddress(m_libraryHandle, symbolName.c_str()))
				return ptr;

			throw Error::Win32Error(Win32::GetLastError(), std::format("Failed to resolve symbol: {}", symbolName));
		}

		void* Resolve(const std::string& symbolName, const std::nothrow_t&) noexcept
		{
			if (not m_libraryHandle)
				return nullptr;
			return Win32::GetProcAddress(m_libraryHandle, symbolName.c_str());
		}

		const std::wstring& GetPath() const noexcept
		{
			return m_path;
		}

		Win32::HMODULE GetHandle() const noexcept
		{
			return m_libraryHandle;
		}

		bool IsLoaded() const noexcept
		{
			return m_libraryHandle != nullptr;
		}
		
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

			m_libraryHandle = Win32::LoadLibraryW(m_path.c_str());
			if (not m_libraryHandle)
				throw Error::Win32Error(Win32::GetLastError(), "failed to load library");
		}

		bool InternalLoad(const std::nothrow_t&) noexcept 
		try
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
		Win32::HMODULE m_libraryHandle = nullptr;
	};
}
