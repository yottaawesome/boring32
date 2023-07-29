export module boring32.msi:database;
import <string>;
import <format>;
import <win32.hpp>;
import boring32.error;

export namespace Boring32::MSI
{
	enum class Mode : uint64_t
	{
		CreateDirect = reinterpret_cast<uint64_t>(MSIDBOPEN_CREATEDIRECT),
		Create = reinterpret_cast<uint64_t>(MSIDBOPEN_CREATE),
		Direct = reinterpret_cast<uint64_t>(MSIDBOPEN_DIRECT),
		ReadOnly = reinterpret_cast<uint64_t>(MSIDBOPEN_READONLY),
		Transact = reinterpret_cast<uint64_t>(MSIDBOPEN_TRANSACT),
		PatchFile = MSIDBOPEN_PATCHFILE
	};

	class Database final
	{
		public:
			~Database() 
			{
				if (m_handle)
					MsiCloseHandle(m_handle);
			};

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

		private:
			void Open()
			{
				if (m_handle)
					MsiCloseHandle(m_handle);

				// https://learn.microsoft.com/en-us/windows/win32/api/msiquery/nf-msiquery-msiopendatabasew
				unsigned status = MsiOpenDatabaseW(
					m_path.c_str(),
					LPCWSTR(m_mode),
					&m_handle
				);
				if (status != ERROR_SUCCESS)
					throw Error::Win32Error("MsiOpenDatabaseW() failed", status);
			}

			std::wstring GetProperty(std::wstring_view propertyName) const
			{
				// See https://stackoverflow.com/questions/27634407/how-to-get-the-product-version-from-an-msi-file-without-installing-the-msi
				PMSIHANDLE hView;
				std::wstring query = std::format(
					L"Select `Value` from `Property` where `Property`='{}'",
					propertyName
				);
				// https://learn.microsoft.com/en-us/windows/win32/api/msiquery/nf-msiquery-msidatabaseopenvieww
				UINT status = MsiDatabaseOpenView(
					m_handle, 
					query.data(), 
					&hView
				);
				if (status != ERROR_SUCCESS)
					throw Error::Win32Error("MsiDatabaseOpenView() failed", status);
				
				// https://learn.microsoft.com/en-us/windows/win32/api/msiquery/nf-msiquery-msiviewexecute
				status = MsiViewExecute(hView, 0);
				if (status != ERROR_SUCCESS)
					throw Error::Win32Error("MsiViewExecute() failed", status);
				
				PMSIHANDLE hViewFetch;
				// https://learn.microsoft.com/en-us/windows/win32/api/msiquery/nf-msiquery-msiviewfetch
				status = MsiViewFetch(hView, &hViewFetch);
				if (status != ERROR_SUCCESS)
					throw Error::Win32Error("MsiViewFetch() failed", status);

				DWORD charCount = 0;
				// https://learn.microsoft.com/en-us/windows/win32/api/msiquery/nf-msiquery-msirecordgetstringw
				std::wstring returnValue;
				status = MsiRecordGetString(
					hViewFetch, 
					1, 
					returnValue.data(), 
					&charCount
				);
				if (status != ERROR_MORE_DATA)
					throw Error::Win32Error("MsiRecordGetString() failed", status);

				charCount++; // must increment to include null-terminator
				returnValue.resize(charCount);
				status = MsiRecordGetString(
					hViewFetch, 
					1, 
					returnValue.data(), 
					&charCount
				);
				if (status != ERROR_SUCCESS)
					throw Error::Win32Error("MsiRecordGetString() failed", status);

				returnValue.resize(charCount);
				return returnValue;
			}

		private:
			std::wstring m_path;
			MSIHANDLE m_handle = 0;
			Mode m_mode = Mode::ReadOnly;
	};
}