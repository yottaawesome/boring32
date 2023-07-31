export module boring32.msi:package;
import <string>;
import <win32.hpp>;
import boring32.error;

export namespace Boring32::MSI
{
	class Package final
	{
		public:
			~Package() { Close(); };
			Package() = delete;
			Package(const Package&) = delete;
			Package& operator=(const Package&) = delete;
			Package(Package&&) noexcept = delete;
			Package& operator=(Package&&) = delete;

		public:
			Package(std::wstring path)
				: m_path(std::move(path))
			{ 
				Open();
			}

		public:
			operator bool() const noexcept { return m_handle; }

		private:
			void Close()
			{
				if (m_handle)
				{
					MsiCloseHandle(m_handle);
					m_handle = 0;
				}
			}

			void Open()
			{
				unsigned status = MsiOpenPackageW(
					m_path.c_str(),
					&m_handle
				);
				if (status != ERROR_SUCCESS)
					throw Error::Win32Error("MsiOpenPackageW() failed", status);
			}

		private:
			std::wstring m_path;
			MSIHANDLE m_handle = 0;
	};
}
