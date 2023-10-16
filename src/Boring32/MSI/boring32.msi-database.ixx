export module boring32.msi:database;
import <string>;
import <memory>;
import <format>;
import boring32.win32;
import boring32.error;

export namespace Boring32::MSI
{
	enum class Mode : uint64_t
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

			Database() = delete;
			// Copyable
			Database(const Database& other) { Copy(other); }
			Database& operator=(const Database& other) { return Copy(other); }
			// Movable
			Database(Database&& other) { Move(other); }
			Database& operator=(Database&& other) noexcept { return Move(other); }

		public:
			Database(std::wstring path, const Mode mode = Mode::ReadOnly)
				: m_path(std::move(path)),
				m_mode(mode)
			{
				Open();
			}

		public:
			operator bool() const noexcept { return m_handle; }

		public:
			std::wstring GetProductVersion() const
			{
				return GetProperty(L"ProductVersion");
			}

			std::wstring GetUpgradeCode() const
			{
				return GetProperty(L"UpgradeCode");
			}

			std::wstring GetProductCode() const
			{
				return GetProperty(L"ProductCode");
			}

			std::wstring GetProductName() const
			{
				return GetProperty(L"ProductName");
			}

			std::wstring GetProductLanguage() const
			{
				return GetProperty(L"ProductLanguage");
			}

			const std::wstring& GetPath() const noexcept
			{
				return m_path;
			}

			const Mode GetMode() const noexcept
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

			Database& Move(Database& other) noexcept
			{
				if (&other == this)
					return *this;

				Close();
				m_handle = other.m_handle;
				other.m_handle = 0;
				m_path = std::move(other.m_path);
				m_mode = other.m_mode;
				return *this;
			}

			Database& Copy(const Database& other)
			{
				if (&other == this)
					return *this;

				Close();
				m_mode = other.m_mode;
				if (other.m_path.empty())
					return *this;
				m_path = other.m_path;
				Open();

				return *this;
			}

			void Open()
			{
				Close();

				// https://learn.microsoft.com/en-us/windows/win32/api/msiquery/nf-msiquery-msiopendatabasew
				unsigned status = Win32::MsiOpenDatabaseW(
					m_path.c_str(),
					LPCWSTR(m_mode),
					&m_handle
				);
				if (status != Win32::ErrorCodes::Success)
					throw Error::Win32Error("MsiOpenDatabaseW() failed", status);
			}

			std::wstring GetProperty(std::wstring_view propertyName) const
			{
				// PMSIHANDLE is actually a RAII-type class, not a pointer.
				// See: https://learn.microsoft.com/en-us/windows/win32/msi/windows-installer-best-practices#use-pmsihandle-instead-of-handle
				Win32::PMSIHANDLE hView;
				// See https://stackoverflow.com/questions/27634407/how-to-get-the-product-version-from-an-msi-file-without-installing-the-msi
				// MSI SQL: https://learn.microsoft.com/en-us/windows/win32/msi/sql-syntax
				// MSI SQL examples: https://learn.microsoft.com/en-us/windows/win32/msi/examples-of-database-queries-using-sql-and-script
				std::wstring query = std::format(
					L"Select `Value` from `Property` where `Property`='{}'",
					propertyName
				);
				// https://learn.microsoft.com/en-us/windows/win32/api/msiquery/nf-msiquery-msidatabaseopenvieww
				UINT status = Win32::MsiDatabaseOpenViewW(
					m_handle, 
					query.data(), 
					&hView
				);
				if (status != Win32::ErrorCodes::Success)
					throw Error::Win32Error("MsiDatabaseOpenView() failed", status);
				
				// https://learn.microsoft.com/en-us/windows/win32/api/msiquery/nf-msiquery-msiviewexecute
				status = Win32::MsiViewExecute(hView, 0);
				if (status != Win32::ErrorCodes::Success)
					throw Error::Win32Error("MsiViewExecute() failed", status);
				
				Win32::PMSIHANDLE hViewFetch;
				// https://learn.microsoft.com/en-us/windows/win32/api/msiquery/nf-msiquery-msiviewfetch
				status = Win32::MsiViewFetch(hView, &hViewFetch);
				if (status != Win32::ErrorCodes::Success)
					throw Error::Win32Error("MsiViewFetch() failed", status);

				Win32::DWORD charCount = 0;
				// https://learn.microsoft.com/en-us/windows/win32/api/msiquery/nf-msiquery-msirecordgetstringw
				std::wstring returnValue;
				status = Win32::MsiRecordGetStringW(
					hViewFetch, 
					1, 
					returnValue.data(), 
					&charCount
				);
				if (status != Win32::ErrorCodes::MoreData)
					throw Error::Win32Error("MsiRecordGetString() failed", status);

				charCount++; // must increment to include null-terminator
				returnValue.resize(charCount);
				status = Win32::MsiRecordGetStringW(
					hViewFetch, 
					1, 
					returnValue.data(), 
					&charCount
				);
				if (status != Win32::ErrorCodes::Success)
					throw Error::Win32Error("MsiRecordGetString() failed", status);

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