export module boring32:filesystem.file;
import std;
import :win32;
import :raii;
import :error;
import :filesystem.functions;

export namespace Boring32::FileSystem
{
	class File final
	{
	public:
		~File()
		{
			TryFlush();
		}
		File() = default;
		File(File&&) noexcept = default;
		auto operator=(File&&) noexcept -> File& = default;

		File(const std::wstring& fileName, Win32::DWORD accessMode = Win32::GenericRead | Win32::GenericWrite)
			: m_accessMode{ accessMode }
		{
			InternalOpen(fileName, accessMode);
		}

		void Close()
		{
			Flush();
			m_fileHandle = nullptr;
		}

		auto GetHandle() const noexcept -> Win32::HANDLE
		{
			return *m_fileHandle;
		}

		auto GetFilePath(Win32::DWORD flags = 0) -> std::wstring
		{
			// get the size
			auto outputBuffer = std::wstring{};
			auto size =
				Win32::GetFinalPathNameByHandleW(
					*m_fileHandle,
					outputBuffer.data(),
					0,
					flags
				);
			// Other win32 functions return ERROR_INSUFFICIENT_BUFFER, so GetFinalPathNameByHandleW() returning NotEnoughMemory is unusual.
			if (auto lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::NotEnoughMemory)
				throw Error::Win32Error{ lastError, "GetFinalPathNameByHandleW() failed" };
			// Returns the size of the buffer, including the null terminator.
			outputBuffer.resize(size-1);
			size = 
				Win32::GetFinalPathNameByHandleW(
					*m_fileHandle,
					outputBuffer.data(),
					size,
					0
				);
			if (size == 0)
				throw Error::Win32Error{ Win32::GetLastError(), "GetFinalPathNameByHandleW() failed" };
			// does not include the null terminator, so size is the number of characters in the path.
			outputBuffer.resize(size);
			return outputBuffer;
		}

		void WriteFile(std::ranges::range auto&& toWrite)
		{
			if (not m_fileHandle)
				throw Error::Boring32Error{ "File is not open" };
			if (toWrite.empty())
				return;
			auto numberOfBytesWritten = Win32::DWORD{};
			::Boring32::FileSystem::WriteFile(*m_fileHandle, toWrite.data(), static_cast<Win32::DWORD>(toWrite.size()), &numberOfBytesWritten);
			if (numberOfBytesWritten != toWrite.size())
				throw Error::Boring32Error{ "Could not write all bytes to file" };
		}

		void SetFilePointer(std::int64_t distanceToMove, Win32::FilePointerMoveMethod moveMethod)
		{
			if (not m_fileHandle)
				throw Error::Boring32Error{ "File is not open" };

			auto lowDistance = static_cast<Win32::DWORD>(distanceToMove & 0xFFFFFFFF);
			auto highDistance = static_cast<Win32::LONG>(distanceToMove >> 32);

			auto success = Win32::SetFilePointer(*m_fileHandle, lowDistance, &highDistance, static_cast<Win32::DWORD>(moveMethod));
			if (success == Win32::InvalidSetFilePointer)
				throw Error::Win32Error{ Win32::GetLastError(), "SetFilePointer() failed" };
		}

		void Clear()
		{
			SetFilePointer(0, Win32::FilePointerMoveMethod::Begin);
			SetEndOfFile();
		}

		auto TryFlush() noexcept -> std::optional<Win32::DWORD>
		{
			if (m_fileHandle and m_accessMode & Win32::GenericWrite and not Win32::FlushFileBuffers(*m_fileHandle))
				return Win32::GetLastError();
			return std::nullopt;
		}

		void Flush()
		{
			if (auto code = TryFlush(); code)
				throw Error::Win32Error{ code.value(), "FlushFileBuffers() failed" };
		}

		void SetEndOfFile()
		{
			if (not m_fileHandle)
				throw Error::Boring32Error{ "File is not open" };
			if (not Win32::SetEndOfFile(*m_fileHandle))
				throw Error::Win32Error{ Win32::GetLastError(), "SetEndOfFile() failed" };
		}

		auto ReadFile(Win32::DWORD numberOfBytesToRead) -> std::vector<std::byte>
		{
			if (not m_fileHandle)
				throw Error::Boring32Error{ "File is not open" };
			if (numberOfBytesToRead == 0)
				return {};
			auto buffer = std::vector<std::byte>(numberOfBytesToRead);
			auto numberOfBytesRead = ::Boring32::FileSystem::ReadFile(*m_fileHandle, buffer.data(), numberOfBytesToRead);
			buffer.resize(numberOfBytesRead);
			return buffer;
		}

		auto GetSize() const -> Win32::DWORD
		{
			// see also GetFileSizeEx 
			if (not m_fileHandle)
				throw Error::Boring32Error{ "File is not open" };
			auto fileSize = Win32::GetFileSize(*m_fileHandle, nullptr);
			if (fileSize == Win32::InvalidFileSize)
				throw Error::Win32Error{ Win32::GetLastError(), "GetFileSize() failed" };
			return fileSize;
		}

		auto GetType() const -> Win32::FileType
		{
			if (not m_fileHandle)
				throw Error::Boring32Error{ "File is not open" };
			auto fileType = Win32::GetFileType(*m_fileHandle);
			if (static_cast<Win32::FileType>(fileType) == Win32::FileType::Unknown)
				if (auto lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::Success)
					throw Error::Win32Error{ lastError, "GetFileType() failed" };
			return static_cast<Win32::FileType>(fileType);
		}

		auto GetAccessMode() const noexcept -> Win32::DWORD
		{
			return m_accessMode;
		}

	private:
		void InternalOpen(const std::wstring& fileName, Win32::DWORD accessMode)
		{
			if (fileName.empty())
				throw Error::Boring32Error{ "Filename must be specified" };

			// https://learn.microsoft.com/en-us/windows/win32/secauthz/generic-access-rights
			// https://learn.microsoft.com/en-us/windows/win32/fileio/file-security-and-access-rights
			// https://learn.microsoft.com/en-us/windows/win32/fileio/file-access-rights-constants
			m_fileHandle = 
				Win32::CreateFileW(
					fileName.c_str(),
					accessMode,
					0,
					nullptr,
					Win32::OpenAlways,
					Win32::FileAttributeNormal,
					nullptr
				);
			if (m_fileHandle == Win32::InvalidHandleValue)
				throw Error::Win32Error{Win32::GetLastError(), "CreateFileW() failed"};
		}

		Win32::DWORD m_accessMode{};
		RAII::UniqueHandle m_fileHandle;
	};
}