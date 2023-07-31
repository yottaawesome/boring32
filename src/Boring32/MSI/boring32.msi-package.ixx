export module boring32.msi:package;
import <win32.hpp>;
import <string>;

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
			{ }

		private:
			void Close()
			{
				if (m_handle)
				{
					MsiCloseHandle(m_handle);
					m_handle = 0;
				}
			}

		private:
			std::wstring m_path;
			MSIHANDLE m_handle = 0;
	};
}
