export module boring32:filesystem.file;
import std;
import :win32;
import :raii;
import :error;

export namespace Boring32::FileSystem
{
	class File final
	{
	public:
		File() = default;
		File(File&&) noexcept = default;
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

		auto GetHandle() const noexcept -> Win32::HANDLE
		{
			return *m_fileHandle;
		}

		auto GetFileName(this auto&& self) noexcept -> decltype(auto)
		{
			return std::forward_like<decltype(self)>(self.m_fileName);
		}

		auto GetSize() const -> Win32::DWORD
		{
			// see also GetFileSizeEx 
			if (not m_fileHandle)
				throw Error::Boring32Error{ "File is not open" };
			auto fileSize = Win32::GetFileSize(*m_fileHandle, nullptr);
			if (fileSize == Win32::InvalidFileSize)
				throw Error::Win32Error{ Win32::GetLastError(), "GetFileSize() failed" };
			return fileSize;
		}

		auto GetType() -> Win32::FileType
		{
			if (not m_fileHandle)
				throw Error::Boring32Error{ "File is not open" };
			auto fileType = Win32::GetFileType(*m_fileHandle);
			if (static_cast<Win32::FileType>(fileType) == Win32::FileType::Unknown)
				if (auto lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::Success)
					throw Error::Win32Error{ lastError, "GetFileType() failed" };
			return static_cast<Win32::FileType>(fileType);
		}

	private:
		void InternalOpen()
		{
			if (m_fileName.empty())
				throw Error::Boring32Error("Filename must be specified");

			// https://learn.microsoft.com/en-us/windows/win32/secauthz/generic-access-rights
			// https://learn.microsoft.com/en-us/windows/win32/fileio/file-security-and-access-rights
			// https://learn.microsoft.com/en-us/windows/win32/fileio/file-access-rights-constants
			m_fileHandle = 
				Win32::CreateFileW(
					m_fileName.c_str(),				// lpFileName
					Win32::GenericRead | Win32::GenericWrite,	// dwDesiredAccess
					0,								// dwShareMode
					nullptr,						// lpSecurityAttributes
					Win32::OpenAlways,					// dwCreationDisposition
					Win32::FileAttributeNormal,			// dwFlagsAndAttributes
					nullptr							// hTemplateFile
				);
			if (m_fileHandle == Win32::InvalidHandleValue)
				throw Error::Win32Error{Win32::GetLastError(), "CreateFileW() failed"};
		}

		std::wstring m_fileName;
		RAII::UniqueHandle m_fileHandle;
	};
}