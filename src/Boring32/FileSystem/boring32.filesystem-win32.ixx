module;

#include <source_location>

export module boring32.filesystem:win32;
import "win32.hpp";
import <string>;

export namespace Boring32::FileSystem::Win32
{
	HANDLE CreateFile(
		const std::wstring& fileName,
		const DWORD desiredAccess,
		const DWORD shareMode,
		SECURITY_ATTRIBUTES* securityAttributes,
		const DWORD creationDisposition,
		const DWORD flagsAndAttributes,
		const HANDLE templateFile
	);
}