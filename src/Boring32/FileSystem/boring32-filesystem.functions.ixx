export module boring32:filesystem.functions;
import std;
import boring32.win32;
import :error;
import :strings;

export namespace Boring32::FileSystem
{
	// Requires linking with Version.lib
	auto GetFileVersion(const std::wstring& filePath) -> std::wstring
	{
		// Adapted from https://stackoverflow.com/questions/940707/how-do-i-programmatically-get-the-version-of-a-dll-or-exe-file
		// See also https://docs.microsoft.com/en-us/windows/win32/menurc/version-information
		if (not std::filesystem::exists(filePath))
			throw Error::Boring32Error(
				"File {} does not exist",
				std::source_location::current(),
				std::stacktrace::current(),
				Strings::ConvertString(filePath)
			);

		Win32::DWORD verHandle = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/winver/nf-winver-getfileversioninfosizew
		Win32::DWORD verSize = Win32::GetFileVersionInfoSizeW(filePath.c_str(), &verHandle);
		if (not verSize)
			throw Error::Win32Error(Win32::GetLastError(), "GetFileVersionInfoSizeW() failed");

		std::vector<std::byte> verData(verSize);
		// https://docs.microsoft.com/en-us/windows/win32/api/winver/nf-winver-getfileversioninfow
		bool success = Win32::GetFileVersionInfoW(
			filePath.c_str(), 
			verHandle, 
			verSize, 
			&verData[0]
		);
		if (not success)
			throw Error::Win32Error(Win32::GetLastError(), "GetFileVersionInfoW() failed");

		unsigned size = 0;
		std::byte* lpBuffer = nullptr; // Free when pBlock argument is freed
		// https://docs.microsoft.com/en-us/windows/win32/api/winver/nf-winver-verqueryvaluew
		success = Win32::VerQueryValueW(
			&verData[0], 
			L"\\", 
			reinterpret_cast<void**>(&lpBuffer), 
			&size
		);
		if (not success)
			throw Error::Boring32Error("Could not determine version info (VerQueryValueW() failed)");
		if (not size)
			throw Error::Boring32Error("Could not determine version info (size was zero)");

		//https://docs.microsoft.com/en-us/windows/win32/api/verrsrc/ns-verrsrc-vs_fixedfileinfo
		Win32::VS_FIXEDFILEINFO* verInfo = reinterpret_cast<Win32::VS_FIXEDFILEINFO*>(lpBuffer);
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
		bool succeeded = Win32::CreateDirectoryW(path.c_str(), nullptr);
		if (not succeeded)
			throw Error::Win32Error(Win32::GetLastError(), "CreateDirectoryW() failed");
	}

	void CreateFileDirectory(const std::wstring& path, const std::wstring& dacl)
	{
		if (path.empty())
			throw Error::Boring32Error("path cannot be empty");
		if (dacl.empty())
			throw Error::Boring32Error("dacl cannot be empty");

		void* sd = nullptr;
		bool succeeded = Win32::ConvertStringSecurityDescriptorToSecurityDescriptorW(
			dacl.c_str(),
			Win32::SddlRevision1, // Must be SDDL_REVISION_1
			&sd,
			nullptr
		);
		if (not succeeded)
			throw Error::Win32Error(Win32::GetLastError(), "ConvertStringSecurityDescriptorToSecurityDescriptorW() failed");
		std::unique_ptr<void, void(*)(void*)> sdDeleter(sd, [](void* ptr) { Win32::LocalFree(ptr); });

		Win32::SECURITY_ATTRIBUTES sa{
			.nLength = sizeof(Win32::SECURITY_ATTRIBUTES),
			.lpSecurityDescriptor = sd
		};
		if (not Win32::CreateDirectoryW(path.c_str(), &sa))
			throw Error::Win32Error(Win32::GetLastError(), "CreateDirectoryW() failed");
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
		if (not Win32::MoveFileExW(oldFile.c_str(), newFile.c_str(), flags))
			throw Error::Win32Error(Win32::GetLastError(), "MoveFileExW() failed");
	}

	auto FileCreate(
		const std::wstring& fileName,
		const Win32::DWORD desiredAccess = Win32::GenericRead | Win32::GenericWrite,
		const Win32::DWORD shareMode = 0,
		Win32::SECURITY_ATTRIBUTES* const securityAttributes = nullptr,
		const Win32::DWORD creationDisposition = Win32::OpenAlways,
		const Win32::DWORD flagsAndAttributes = Win32::FileAttributeNormal,
		const Win32::HANDLE templateFile = nullptr
	) -> Win32::HANDLE
	{
		if (fileName.empty())
			throw Error::Boring32Error("Filename must be specified");

		// https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew
		Win32::HANDLE fileHandle = Win32::CreateFileW(
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
		if (fileHandle == Win32::InvalidHandleValue)
			throw Error::Win32Error(Win32::GetLastError(), "CreateFileW() failed");
		return fileHandle;
	}

	void WriteFile(
		const Win32::HANDLE file,
		const void* lpBuffer,
		Win32::DWORD numberOfBytesToWrite,
		Win32::DWORD* const numberOfBytesWritten
	)
	{
		if (not file)
			throw Error::Boring32Error("File handle cannot be null");
		if (file == Win32::InvalidHandleValue)
			throw Error::Boring32Error("File handle cannot be INVALID_HANDLE_VALUE");
		if (not numberOfBytesWritten)
			throw Error::Boring32Error("numberOfBytesWritten cannot be null");
		if (numberOfBytesToWrite == 0)
		{
			*numberOfBytesWritten = 0;
			return;
		}
		if (not lpBuffer)
			throw Error::Boring32Error("Buffer cannot be null");

		// https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
		bool success = Win32::WriteFile(
			file,
			lpBuffer,
			numberOfBytesToWrite,
			numberOfBytesWritten,
			nullptr
		);
		if (not success)
			throw Error::Win32Error(Win32::GetLastError(), "WriteFile() failed");
	}

	void WriteFile(
		const Win32::HANDLE file,
		const void* lpBuffer,
		Win32::DWORD numberOfBytesToWrite,
		const Win32::OVERLAPPED& overlapped
	)
	{
		if (not file)
			throw Error::Boring32Error("File handle cannot be null");
		if (file == Win32::InvalidHandleValue)
			throw Error::Boring32Error("File handle cannot be INVALID_HANDLE_VALUE");
		if (numberOfBytesToWrite == 0)
			return;
		
		if (not lpBuffer)
			throw Error::Boring32Error("Buffer cannot be null");

		// https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
		bool success = Win32::WriteFile(
			file,
			lpBuffer,
			numberOfBytesToWrite,
			nullptr,
			const_cast<Win32::OVERLAPPED*>(&overlapped)
		);
		if (not success)
			throw Error::Win32Error(Win32::GetLastError(), "WriteFile() failed");
	}

	void ReadFile(
		const Win32::HANDLE file,
		void* const lpBuffer,
		Win32::DWORD nNumberOfBytesToRead,
		Win32::DWORD& lpNumberOfBytesRead
	)
	{
		if (not file)
			throw Error::Boring32Error("File handle cannot be null");
		if (file == Win32::InvalidHandleValue)
			throw Error::Boring32Error("File handle cannot be INVALID_HANDLE_VALUE");
		if (nNumberOfBytesToRead == 0)
			return;
		if (not lpBuffer)
			throw Error::Boring32Error("Buffer cannot be null");

		bool success = Win32::ReadFile(
			file,
			lpBuffer,
			nNumberOfBytesToRead,
			&lpNumberOfBytesRead,
			nullptr
		);
		if (not success)
			throw Error::Win32Error(Win32::GetLastError(), "ReadFile() failed");
	}
}