export module boring32:msi.database;
import std;
import :win32;
import :error;

export namespace Boring32::MSI
{
	enum class Mode : std::uint64_t
	{
		CreateDirect = Win32::MsiDbOpen_CreateDirect<uint64_t>(),
		Create = Win32::MsiDbOpen_Create<uint64_t>(),
		Direct = Win32::MsiDbOpen_Direct<uint64_t>(),
		ReadOnly = Win32::MsiDbOpen_ReadOnly<uint64_t>(),
		Transact = Win32::MsiDbOpen_Transact<uint64_t>(),
		PatchFile = Win32::MsiDbOpen_PatchFile
	};

	class Database final
	{
	public:
		~Database() 
		{
			Close();
		};

		// Not copyable
		Database(const Database& other) = delete;
		auto operator=(const Database& other) -> Database& = delete;
		// Movable
		Database(Database&& other) 
		{ 
			Move(other); 
		}
		auto operator=(Database&& other) noexcept -> Database&
		{ 
			return Move(other); 
		}

		Database(std::wstring path, Mode mode = Mode::ReadOnly)
			: m_path(std::move(path))
			, m_mode(mode)
		{
			Open();
		}

		constexpr operator bool() const noexcept { return m_handle; }

		auto GetProductVersion() const -> std::wstring
		{
			return GetProperty(L"ProductVersion");
		}

		auto GetUpgradeCode() const -> std::wstring
		{
			return GetProperty(L"UpgradeCode");
		}

		auto GetProductCode() const -> std::wstring
		{
			return GetProperty(L"ProductCode");
		}

		auto GetProductName() const -> std::wstring
		{
			return GetProperty(L"ProductName");
		}

		auto GetProductLanguage() const -> std::wstring
		{
			return GetProperty(L"ProductLanguage");
		}

		auto GetPath() const noexcept -> std::wstring
		{
			return m_path;
		}

		auto GetMode() const noexcept -> Mode
		{
			return m_mode;
		}

	private:
		void Close() noexcept
		{
			if (m_handle)
			{
				Win32::MsiCloseHandle(m_handle);
				m_handle = 0;
			}
		}

		auto Move(Database& other) noexcept -> Database&
		{
			if (&other == this)
				return *this;

			Close();
			m_handle = std::exchange(other.m_handle, 0);
			m_path = std::move(other.m_path);
			m_mode = other.m_mode;
			return *this;
		}

		void Open()
		{
			Close();

			// https://learn.microsoft.com/en-us/windows/win32/api/msiquery/nf-msiquery-msiopendatabasew
			auto status = Win32::UINT{ Win32::MsiOpenDatabaseW(m_path.c_str(), Win32::LPCWSTR(m_mode), &m_handle) };
			if (status != Win32::ErrorCodes::Success)
				throw Error::Win32Error{status, "MsiOpenDatabaseW() failed"};
		}

		auto GetProperty(std::wstring_view propertyName) const -> std::wstring
		{
			// PMSIHANDLE is actually a RAII-type class, not a pointer.
			// See: https://learn.microsoft.com/en-us/windows/win32/msi/windows-installer-best-practices#use-pmsihandle-instead-of-handle
			auto hView = Win32::PMSIHANDLE{};
			// See https://stackoverflow.com/questions/27634407/how-to-get-the-product-version-from-an-msi-file-without-installing-the-msi
			// MSI SQL: https://learn.microsoft.com/en-us/windows/win32/msi/sql-syntax
			// MSI SQL examples: https://learn.microsoft.com/en-us/windows/win32/msi/examples-of-database-queries-using-sql-and-script
			auto query = std::format(L"Select `Value` from `Property` where `Property`='{}'", propertyName);
			// https://learn.microsoft.com/en-us/windows/win32/api/msiquery/nf-msiquery-msidatabaseopenvieww
			auto status = Win32::MsiDatabaseOpenViewW(m_handle, query.data(), &hView);
			if (status != Win32::ErrorCodes::Success)
				throw Error::Win32Error{status, "MsiDatabaseOpenView() failed"};
				
			// https://learn.microsoft.com/en-us/windows/win32/api/msiquery/nf-msiquery-msiviewexecute
			status = Win32::MsiViewExecute(hView, 0);
			if (status != Win32::ErrorCodes::Success)
				throw Error::Win32Error{status, "MsiViewExecute() failed"};
				
			auto hViewFetch = Win32::PMSIHANDLE{};
			// https://learn.microsoft.com/en-us/windows/win32/api/msiquery/nf-msiquery-msiviewfetch
			status = Win32::MsiViewFetch(hView, &hViewFetch);
			if (status != Win32::ErrorCodes::Success)
				throw Error::Win32Error{status, "MsiViewFetch() failed"};

			auto charCount = Win32::DWORD{0};
			// https://learn.microsoft.com/en-us/windows/win32/api/msiquery/nf-msiquery-msirecordgetstringw
			auto returnValue = std::wstring{};
			status = Win32::MsiRecordGetStringW(
				hViewFetch, 
				1, 
				returnValue.data(), 
				&charCount
			);
			if (status != Win32::ErrorCodes::MoreData)
				throw Error::Win32Error{status, "MsiRecordGetString() failed"};

			charCount++; // must increment to include null-terminator
			returnValue.resize(charCount);
			status = Win32::MsiRecordGetStringW(
				hViewFetch, 
				1, 
				returnValue.data(), 
				&charCount
			);
			if (status != Win32::ErrorCodes::Success)
				throw Error::Win32Error{status, "MsiRecordGetString() failed"};

			returnValue.resize(charCount);
			return returnValue;
		}

	private:
		std::wstring m_path;
		// Given this is just an int, unique_ptr doesn't play
		// nicely with it.
		Win32::MSIHANDLE m_handle = 0;
		Mode m_mode = Mode::ReadOnly;
	};
}