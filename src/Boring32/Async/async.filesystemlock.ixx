export module boring32:async.filesystemlock;
import std;
import :win32;
import :concepts;
import :error;
import :raii;
import :strings;

export namespace Boring32::Async
{
	// Uses a file lock to provide inter-process locking.
	// Not effective on the same process, use FileRangeLock or 
	// other synchronization primitives for that.
	template<Strings::FixedStringW VPath = L"FileLock.lock">
	class FileSystemLock
	{
	public:
		constexpr FileSystemLock() = default;

		FileSystemLock(bool acquire)
		{
			if (acquire) lock();
		}

		FileSystemLock(const FileSystemLock&) = delete;
		auto operator=(const FileSystemLock&) -> FileSystemLock& = delete;

		FileSystemLock(FileSystemLock&&) = default;
		auto operator=(FileSystemLock&&) -> FileSystemLock& = default;

		void lock(this auto&& self)
		{
			// Already owned
			if (self.fileHandle)
				return;

			auto handle = Win32::CreateFileW(
				VPath.ToView().data(),
				Win32::GenericRead,
				static_cast<Win32::DWORD>(Win32::FileShareMode::None),
				nullptr,
				static_cast<Win32::DWORD>(Win32::CreateFileDisposition::CreateNew),
				Win32::FileAttributes::Normal | Win32::FileFlags::DeleteOnClose, // can also use hidden
				nullptr
			);
			if (not handle or handle == Win32::InvalidHandleValue)
				throw Error::Win32Error{ Win32::GetLastError(), "Failed to create or open file handle."};
			self.fileHandle = RAII::HandleUniquePtr(handle);
		}

		void unlock(this auto&& self) noexcept
		{
			if (not self.fileHandle)
				return;
			self.fileHandle.reset();
		}

		auto try_lock(this auto&& self) noexcept -> bool
		try
		{
			return (self.lock(), true);
		}
		catch (...)
		{
			return false;
		}

		constexpr auto IsLocked(this const FileSystemLock& self) noexcept -> bool
		{
			return self.fileHandle != nullptr;
		}

		explicit constexpr operator bool(this const FileSystemLock& self) noexcept
		{
			return self.IsLocked();
		}

	private:
		auto Move(this FileSystemLock& self, FileSystemLock& other) -> FileSystemLock&
		{
			return (self.fileHandle = std::move(other.fileHandle), self);
		}
		RAII::HandleUniquePtr fileHandle;
	};
	static_assert(Concepts::Lockable<FileSystemLock<>>);
}
	