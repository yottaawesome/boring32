module;

#include <string>
#include <stdexcept>
#include <source_location>
#include <windows.h>

module boring32.filesystem:file;
import boring32.error;

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

	void File::InternalOpen()
	{
		if (m_fileName.empty())
			throw Error::Boring32Error("Filename must be specified");

		m_fileHandle = CreateFileW(
			m_fileName.c_str(),				// lpFileName
			GENERIC_READ | GENERIC_WRITE,	// dwDesiredAccess
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