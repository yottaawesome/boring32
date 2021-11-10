module;

#include <string>
#include <windows.h>
#include "include/Error/Win32Error.hpp"

module boring32.filesystem.file;

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
			throw std::runtime_error(__FUNCSIG__": filename must be specified");

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
			throw Error::Win32Error(__FUNCSIG__": CreateFileW() failed", GetLastError());
	}
}