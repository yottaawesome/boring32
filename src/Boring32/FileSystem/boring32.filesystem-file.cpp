module boring32.filesystem:file;
import boring32.error;
import <source_location>; // importing causes ICE
import <win32.hpp>;

namespace Boring32::FileSystem
{
	File::~File() { Close(); }

	File::File(){}

	File::File(std::wstring fileName)
		:m_fileName(std::move(fileName))
	{
		InternalOpen();
	}

	void File::Close()
	{
		m_fileHandle = nullptr;
	}

	HANDLE File::GetHandle() const noexcept
	{
		return *m_fileHandle;
	}

	void File::InternalOpen()
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
}