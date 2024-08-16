export module boring32:msi_package;
import boring32.shared;
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
					Win32::MsiCloseHandle(m_handle);
					m_handle = 0;
				}
			}

			void Open()
			{
				// https://learn.microsoft.com/en-us/windows/win32/api/msi/nf-msi-msiopenpackagew
				// https://learn.microsoft.com/en-us/windows/win32/api/msi/nf-msi-msiopenpackageexw
				unsigned status = Win32::MsiOpenPackageW(
					m_path.c_str(),
					&m_handle
				);
				if (status != Win32::ErrorCodes::Success)
					throw Error::Win32Error("MsiOpenPackageW() failed", status);
			}

		private:
			std::wstring m_path;
			Win32::MSIHANDLE m_handle = 0;
	};
}
