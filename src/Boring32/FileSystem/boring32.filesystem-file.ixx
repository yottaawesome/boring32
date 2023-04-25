export module boring32.filesystem:file;
import boring32.raii;
import boring32.error;
import <string>;
import <win32.hpp>;

export namespace Boring32::FileSystem
{
	class File
	{
		public:
			virtual ~File() { Close(); }

			File() = default;

			File(std::wstring fileName)
				:m_fileName(std::move(fileName))
			{
				InternalOpen();
			}

		public:
			virtual void Close()
			{
				m_fileHandle = nullptr;
			}

			virtual HANDLE GetHandle() const noexcept
			{
				return *m_fileHandle;
			}

		protected:
			virtual void InternalOpen()
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

		protected:
			std::wstring m_fileName;
			RAII::Win32Handle m_fileHandle;
	};
}