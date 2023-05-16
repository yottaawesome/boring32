module;

#include <source_location>;

export module boring32.filesystem:file;
import <string>;
import <win32.hpp>;
import boring32.raii;
import boring32.error;

export namespace Boring32::FileSystem
{
	class File final
	{
		public:
			~File() = default;
			File() = default;
			File(const File&) = default;
			File(File&&) noexcept = default;
			File& operator=(const File&) = default;
			File& operator=(File&&) noexcept = default;

		public:
			File(std::wstring fileName)
				: m_fileName(std::move(fileName))
			{
				InternalOpen();
			}

		public:
			void Close()
			{
				m_fileHandle = nullptr;
			}

			HANDLE GetHandle() const noexcept
			{
				return *m_fileHandle;
			}

		private:
			void InternalOpen()
			{
				if (m_fileName.empty())
					throw Error::Boring32Error("Filename must be specified");

				m_fileHandle = CreateFileW(
					m_fileName.c_str(),				// lpFileName
					GENERIC_READ | GENERIC_WRITE,	// dwDesiredAccess
					// https://learn.microsoft.com/en-us/windows/win32/secauthz/generic-access-rights
					// https://learn.microsoft.com/en-us/windows/win32/fileio/file-security-and-access-rights
					// https://learn.microsoft.com/en-us/windows/win32/fileio/file-access-rights-constants
					0,								// dwShareMode
					nullptr,						// lpSecurityAttributes
					OPEN_ALWAYS,					// dwCreationDisposition
					FILE_ATTRIBUTE_NORMAL,			// dwFlagsAndAttributes
					nullptr							// hTemplateFile
				);
				if (m_fileHandle == INVALID_HANDLE_VALUE)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("CreateFileW() failed", lastError);
				}
			}

		private:
			std::wstring m_fileName;
			RAII::Win32Handle m_fileHandle;
	};
}