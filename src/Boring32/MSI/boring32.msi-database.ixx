export module boring32.msi:database;
import <string>;
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

		private:
			std::wstring m_path;
			MSIHANDLE m_handle = 0;
			Mode m_mode = Mode::ReadOnly;
	};
}