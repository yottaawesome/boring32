module;

#include <source_location>

export module boring32.filesystem:win32;
import "win32.hpp";
import <string>;

export namespace Boring32::FileSystem::Win32
{
	HANDLE FileCreate(
		const std::wstring& fileName,
		const DWORD desiredAccess = GENERIC_READ | GENERIC_WRITE,
		const DWORD shareMode = 0,
		SECURITY_ATTRIBUTES* const securityAttributes = nullptr,
		const DWORD creationDisposition = OPEN_ALWAYS,
		const DWORD flagsAndAttributes = FILE_ATTRIBUTE_NORMAL,
		const HANDLE templateFile = nullptr,
		const std::source_location& location = std::source_location::current()
	);

	void WriteFile(
		const HANDLE file,
		const void* lpBuffer,
		const DWORD numberOfBytesToWrite,
		DWORD* const numberOfBytesWritten,
		const std::source_location& location = std::source_location::current()
	);
	void WriteFile(
		const HANDLE file,
		const void* lpBuffer,
		const DWORD numberOfBytesToWrite,
		const OVERLAPPED& overlapped,
		const std::source_location& location = std::source_location::current()
	);
	void ReadFile(
		const HANDLE file,
		void* const lpBuffer,
		const DWORD nNumberOfBytesToRead,
		DWORD& lpNumberOfBytesRead,
		const std::source_location& location = std::source_location::current()
	);
}