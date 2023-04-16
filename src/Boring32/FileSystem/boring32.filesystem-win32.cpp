module boring32.filesystem:win32;
import "win32.hpp";
import <string>;
import boring32.error;

namespace Boring32::FileSystem::Win32
{
	HANDLE CreateFile(
		const std::wstring& fileName,
		const DWORD desiredAccess,
		const DWORD shareMode,
		SECURITY_ATTRIBUTES* securityAttributes,
		const DWORD creationDisposition,
		const DWORD flagsAndAttributes,
		const HANDLE templateFile
	)
	{
		if (fileName.empty())
			throw Error::Boring32Error("Filename must be specified");

		const HANDLE fileHandle = CreateFileW(
			fileName.c_str(),				// lpFileName
			desiredAccess,	// dwDesiredAccess
			// https://learn.microsoft.com/en-us/windows/win32/secauthz/generic-access-rights
			// https://learn.microsoft.com/en-us/windows/win32/fileio/file-security-and-access-rights
			// https://learn.microsoft.com/en-us/windows/win32/fileio/file-access-rights-constants
			shareMode,								// dwShareMode
			securityAttributes,						// lpSecurityAttributes
			creationDisposition,					// dwCreationDisposition
			flagsAndAttributes,			// dwFlagsAndAttributes
			templateFile							// hTemplateFile
		);
		if (fileHandle == INVALID_HANDLE_VALUE)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("CreateFileW() failed", lastError);
		}
		return fileHandle;
	}
}