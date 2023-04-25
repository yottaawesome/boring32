export module boring32.filesystem:functions;
import <string>;
import <source_location>;
import boring32.error;
import boring32.strings;
import <filesystem>;
import <format>;
import <vector>;
import <memory>;
import <win32.hpp>;

export namespace Boring32::FileSystem
{
	// Requires linking with Version.lib
	std::wstring GetFileVersion(const std::wstring& filePath)
	{
		// Adapted from https://stackoverflow.com/questions/940707/how-do-i-programmatically-get-the-version-of-a-dll-or-exe-file
		// See also https://docs.microsoft.com/en-us/windows/win32/menurc/version-information
		if (!std::filesystem::exists(filePath))
			throw Error::Boring32Error(
				std::format(
					"File {} does not exist",
					Strings::ConvertString(filePath)
				)
			);

		DWORD verHandle = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/winver/nf-winver-getfileversioninfosizew
		const DWORD verSize = GetFileVersionInfoSizeW(filePath.c_str(), &verHandle);
		if (!verSize)
			throw Error::Win32Error("GetFileVersionInfoSizeW() failed", GetLastError());

		std::vector<std::byte> verData(verSize);
		// https://docs.microsoft.com/en-us/windows/win32/api/winver/nf-winver-getfileversioninfow
		if (!GetFileVersionInfoW(filePath.c_str(), verHandle, verSize, &verData[0]))
			throw Error::Win32Error("GetFileVersionInfoW() failed", GetLastError());

		UINT size = 0;
		LPBYTE lpBuffer = nullptr;
		// https://docs.microsoft.com/en-us/windows/win32/api/winver/nf-winver-verqueryvaluew
		if (!VerQueryValueW(&verData[0], L"\\", reinterpret_cast<void**>(&lpBuffer), &size))
			throw Error::Boring32Error("Could not determine version info (VerQueryValueW() failed)");
		if (!size)
			throw Error::Boring32Error("Could not determine version info (size was zero)");

		//https://docs.microsoft.com/en-us/windows/win32/api/verrsrc/ns-verrsrc-vs_fixedfileinfo
		const VS_FIXEDFILEINFO* verInfo = reinterpret_cast<VS_FIXEDFILEINFO*>(lpBuffer);
		if (verInfo->dwSignature != 0xfeef04bd)
			throw Error::Boring32Error("Could not determine version info (invalid signature)");

		// Doesn't matter if you are on 32 bit or 64 bit,
		// DWORD is always 32 bits, so first two revision numbers
		// come from dwFileVersionMS, last two come from dwFileVersionLS
		return std::format(
			L"{}.{}.{}.{}", // major.minor.build.revision
			(verInfo->dwFileVersionMS >> 16) & 0xffff,
			(verInfo->dwFileVersionMS >> 0) & 0xffff,
			(verInfo->dwFileVersionLS >> 16) & 0xffff,
			(verInfo->dwFileVersionLS >> 0) & 0xffff
		);
	}

	// Requires the parent directories to exist
	void CreateFileDirectory(const std::wstring& path)
	{
		if (path.empty())
			throw Error::Boring32Error("path cannot be empty");
		const bool succeeded = CreateDirectoryW(
			path.c_str(),
			nullptr
		);
		if (!succeeded)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("CreateDirectoryW() failed", lastError);
		}
	}

	void CreateFileDirectory(const std::wstring& path, const std::wstring& dacl)
	{
		if (path.empty())
			throw Error::Boring32Error("path cannot be empty");
		if (dacl.empty())
			throw Error::Boring32Error("dacl cannot be empty");

		void* sd = nullptr;
		const bool succeeded = ConvertStringSecurityDescriptorToSecurityDescriptorW(
			dacl.c_str(),
			SDDL_REVISION_1, // Must be SDDL_REVISION_1
			&sd,
			nullptr
		);
		if (!succeeded)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("ConvertStringSecurityDescriptorToSecurityDescriptorW() failed", lastError);
		}
		std::unique_ptr<void, void(*)(void*)> sdDeleter(sd, [](void* ptr) { LocalFree(ptr); });

		SECURITY_ATTRIBUTES sa{
			.nLength = sizeof(SECURITY_ATTRIBUTES),
			.lpSecurityDescriptor = sd
		};
		if (!CreateDirectoryW(path.c_str(), &sa))
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("CreateDirectoryW() failed", lastError);
		}
	}

	void MoveNamedFile(
		const std::wstring& oldFile, 
		const std::wstring& newFile,
		unsigned flags = 0
	)
	{
		if (oldFile.empty())
			throw Error::Boring32Error("oldFile cannot be empty");
		if (newFile.empty())
			throw Error::Boring32Error("newFile cannot be empty");

		// https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-movefileexw
		if (!MoveFileEx(oldFile.c_str(), newFile.c_str(), flags))
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("MoveFileEx() failed", lastError);
		}
	}

	HANDLE FileCreate(
		const std::wstring& fileName,
		const DWORD desiredAccess = GENERIC_READ | GENERIC_WRITE,
		const DWORD shareMode = 0,
		SECURITY_ATTRIBUTES* const securityAttributes = nullptr,
		const DWORD creationDisposition = OPEN_ALWAYS,
		const DWORD flagsAndAttributes = FILE_ATTRIBUTE_NORMAL,
		const HANDLE templateFile = nullptr,
		const std::source_location& location = std::source_location::current()
	)
	{
		if (fileName.empty())
			throw Error::Boring32Error("Filename must be specified");

		// https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew
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
			throw Error::Win32Error("CreateFileW() failed", lastError, location);
		}
		return fileHandle;
	}

	void WriteFile(
		const HANDLE file,
		const void* lpBuffer,
		const DWORD numberOfBytesToWrite,
		DWORD* const numberOfBytesWritten,
		const std::source_location& location = std::source_location::current()
	)
	{
		if (!file)
			throw Error::Boring32Error("File handle cannot be null", location);
		if (file == INVALID_HANDLE_VALUE)
			throw Error::Boring32Error("File handle cannot be INVALID_HANDLE_VALUE", location);
		if (!numberOfBytesWritten)
			throw Error::Boring32Error("numberOfBytesWritten cannot be null", location);
		if (numberOfBytesToWrite == 0)
		{
			*numberOfBytesWritten = 0;
			return;
		}
		if (!lpBuffer)
			throw Error::Boring32Error("Buffer cannot be null", location);

		// https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
		const bool success = ::WriteFile(
			file,
			lpBuffer,
			numberOfBytesToWrite,
			numberOfBytesWritten,
			nullptr
		);
		if (!success)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error(
				"WriteFile() failed",
				lastError,
				location
			);
		}
	}

	void WriteFile(
		const HANDLE file,
		const void* lpBuffer,
		const DWORD numberOfBytesToWrite,
		const OVERLAPPED& overlapped,
		const std::source_location& location = std::source_location::current()
	)
	{
		if (!file)
			throw Error::Boring32Error("File handle cannot be null", location);
		if (file == INVALID_HANDLE_VALUE)
			throw Error::Boring32Error("File handle cannot be INVALID_HANDLE_VALUE", location);
		if (numberOfBytesToWrite == 0)
		{
			return;
		}
		if (!lpBuffer)
			throw Error::Boring32Error("Buffer cannot be null", location);

		// https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
		const bool success = ::WriteFile(
			file,
			lpBuffer,
			numberOfBytesToWrite,
			nullptr,
			const_cast<OVERLAPPED*>(&overlapped)
		);
		if (!success)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error(
				"WriteFile() failed",
				lastError,
				location
			);
		}
	}

	void ReadFile(
		const HANDLE file,
		void* const lpBuffer,
		const DWORD nNumberOfBytesToRead,
		DWORD& lpNumberOfBytesRead,
		const std::source_location& location = std::source_location::current()
	)
	{
		if (!file)
			throw Error::Boring32Error("File handle cannot be null", location);
		if (file == INVALID_HANDLE_VALUE)
			throw Error::Boring32Error("File handle cannot be INVALID_HANDLE_VALUE", location);
		if (nNumberOfBytesToRead == 0)
		{
			return;
		}
		if (!lpBuffer)
			throw Error::Boring32Error("Buffer cannot be null", location);

		const bool success = ::ReadFile(
			file,
			lpBuffer,
			nNumberOfBytesToRead,
			&lpNumberOfBytesRead,
			nullptr
		);
		if (!success)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error(
				"ReadFile() failed",
				lastError,
				location
			);
		}
	}
}