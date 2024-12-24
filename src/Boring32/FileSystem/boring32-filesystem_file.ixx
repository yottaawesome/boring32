export module boring32:filesystem_file;
import boring32.shared;
import boring32.win32;
import :raii;
import :error;

export namespace Boring32::FileSystem
{
	struct File final
	{
		File() = default;
		File(const File&) = default;
		File(File&&) noexcept = default;
		File& operator=(const File&) = default;
		File& operator=(File&&) noexcept = default;

		File(std::wstring fileName)
			: m_fileName(std::move(fileName))
		{
			InternalOpen();
		}

		void Close()
		{
			m_fileHandle = nullptr;
		}

		Win32::HANDLE GetHandle() const noexcept
		{
			return *m_fileHandle;
		}

		private:
		void InternalOpen()
		{
			if (m_fileName.empty())
				throw Error::Boring32Error("Filename must be specified");

			m_fileHandle = Win32::CreateFileW(
				m_fileName.c_str(),				// lpFileName
				Win32::GenericRead | Win32::GenericWrite,	// dwDesiredAccess
				// https://learn.microsoft.com/en-us/windows/win32/secauthz/generic-access-rights
				// https://learn.microsoft.com/en-us/windows/win32/fileio/file-security-and-access-rights
				// https://learn.microsoft.com/en-us/windows/win32/fileio/file-access-rights-constants
				0,								// dwShareMode
				nullptr,						// lpSecurityAttributes
				Win32::OpenAlways,					// dwCreationDisposition
				Win32::FileAttributeNormal,			// dwFlagsAndAttributes
				nullptr							// hTemplateFile
			);
			if (auto lastError = Win32::GetLastError(); m_fileHandle == Win32::InvalidHandleValue)
				throw Error::Win32Error("CreateFileW() failed", lastError);
		}

		std::wstring m_fileName;
		RAII::Win32Handle m_fileHandle;
	};
}