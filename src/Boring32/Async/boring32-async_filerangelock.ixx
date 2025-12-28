export module boring32:async_filerangelock;
import std;
import boring32.win32;
import :concepts;
import :error;
import :raii;
import :util;

export namespace Boring32::Async
{
	struct FileRangeLock final
	{
		~FileRangeLock() { Clear(); }
		constexpr FileRangeLock() = default;
		FileRangeLock(const FileRangeLock&) = delete;
		FileRangeLock& operator=(const FileRangeLock&) = delete;

		FileRangeLock(FileRangeLock&& other)
		{
			Move(other);
		}

		FileRangeLock& operator=(this FileRangeLock& self, FileRangeLock&& other)
		{
			self.Move(other);
			return self;
		}

		FileRangeLock(std::filesystem::path path, bool acquire)
			: filePath(std::move(path)) 
		{
			acquire ? OpenHandleAndLock() : CreateOrOpenFileHandle(filePath);
		}

		FileRangeLock(std::filesystem::path path, bool acquire, std::uint64_t byteRange)
			: filePath(std::move(path)), range(byteRange)
		{
			acquire ? OpenHandleAndLock() : CreateOrOpenFileHandle(filePath);
		}

		void lock(this FileRangeLock& self)
		{ 
			self.OpenHandleAndLock(); 
		}
		
		void unlock(this FileRangeLock& self) noexcept
		{ 
			self.DoUnlock(); 
		}

		auto try_lock(this FileRangeLock& self) noexcept -> bool
		try
		{
			self.lock();
			return true;
		}
		catch (...)
		{
			return false;
		}

		constexpr auto HandleIsValid(this const FileRangeLock& self) -> bool
		{
			return self.fileHandle.get() != Win32::InvalidHandleValue and self.fileHandle;
		}

		auto GetPath(this const FileRangeLock& self) -> const std::filesystem::path&
		{
			return self.filePath;
		}

	private:
		std::filesystem::path filePath;
		RAII::HandleUniquePtr fileHandle;
		// 0xFFFFFFFF;
		std::uint64_t range = std::numeric_limits<std::uint64_t>::max();

		void Clear(this FileRangeLock& self)
		{
			self.DoUnlock();
			self.fileHandle.reset();
			self.filePath.clear();
		}

		void Move(this FileRangeLock& self, FileRangeLock& other) noexcept
		{
			self.DoUnlock();
			self.fileHandle = std::move(other.fileHandle);
			self.filePath = std::move(other.filePath);
			self.range = other.range;
		}

		void CreateOrOpenFileHandle(this FileRangeLock& self, const std::filesystem::path& path)
		{
			if (self.fileHandle)
				return;

			const auto disposition = std::filesystem::exists(path)
				? Win32::CreateFileDisposition::OpenExisting
				: Win32::CreateFileDisposition::CreateNew;

			self.fileHandle = RAII::HandleUniquePtr( 
				Win32::CreateFileW(
					path.wstring().c_str(),
					Win32::GenericRead,
					static_cast<Win32::DWORD>(Win32::FileShareMode::Read),
					nullptr,
					static_cast<Win32::DWORD>(disposition),
					Win32::FileAttributes::Normal, // can also use hidden
					nullptr
				));
			if (not self.HandleIsValid())
			{
				const auto lastError = Win32::GetLastError();
				throw Error::Win32Error(lastError, "Failed to create or open file handle.");
			}
		}
		
		void DoLock(this FileRangeLock& self)
		{
			if (not self.HandleIsValid())
				throw Error::Boring32Error("File handle is null.");

			Win32::OVERLAPPED overlapped{};
			constexpr auto options = Win32::LockFileFlags::Exclusive | Win32::LockFileFlags::FailImmediately;
			// Requires generic read or generic write
			const auto [lockSizeLow, lockSizeHigh] = Util::Decompose(self.range);
			auto succeeded = Win32::LockFileEx(
				self.fileHandle.get(),
				options,
				0,
				lockSizeLow,
				lockSizeHigh,
				&overlapped
			);
			if (not succeeded)
			{
				const auto lastError = Win32::GetLastError();
				throw Error::Win32Error(lastError, std::format("Failed to lock file {}.", self.filePath.string()));
			}
		}

		void DoUnlock(this FileRangeLock& self)
		{
			if (not self.HandleIsValid())
				return;
			Win32::OVERLAPPED overlapped{};
			const auto [lockSizeLow, lockSizeHigh] = Util::Decompose(self.range);
			auto succeeded = Win32::UnlockFileEx(
				self.fileHandle.get(),
				0,
				lockSizeLow,
				lockSizeHigh,
				&overlapped
			);
			if (succeeded)
				return;
			if (const auto lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::NotLocked)
				throw Error::Win32Error(lastError, std::format("Failed to unlock file {}.", self.filePath.string()));
		}

		void OpenHandleAndLock(this FileRangeLock& self)
		{
			self.CreateOrOpenFileHandle(self.filePath);
			self.DoLock();
		}
	};
	static_assert(Concepts::Lockable<FileRangeLock>);
}
