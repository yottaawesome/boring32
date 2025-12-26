export module boring32:async_filelock;
import std;
import boring32.win32;
import :concepts;
import :error;
import :raii;

export namespace Boring32::Async
{
	struct FileLock final
	{
		~FileLock() { Clear(); }
		FileLock() = default;
		FileLock(const FileLock&) = delete;
		FileLock& operator=(const FileLock&) = delete;

		FileLock(FileLock&& other)
		{
			Move(other);
		}
		FileLock& operator=(this FileLock& self, FileLock&& other)
		{
			self.Move(other);
			return self;
		}

		FileLock(std::filesystem::path path, bool acquire) 
			: filePath(std::move(path)) 
		{
			acquire ? OpenHandleAndLock() : CreateOrOpenFileHandle(filePath);
		}

		void lock(this FileLock& self) { self.OpenHandleAndLock(); }
		void unlock(this FileLock& self) { self.DoUnlock(); }

		auto HandleIsValid(this const FileLock& self) -> bool
		{
			return self.fileHandle.get() != Win32::InvalidHandleValue and self.fileHandle;
		}

		auto GetPath(this const FileLock& self) -> const std::filesystem::path&
		{
			return self.filePath;
		}

	private:
		std::filesystem::path filePath;
		RAII::HandleUniquePtr fileHandle;

		void Clear(this FileLock& self)
		{
			self.DoUnlock();
			self.fileHandle.reset();
			self.filePath.clear();
		}

		void Move(this FileLock& self, FileLock& other)
		{
			self.DoUnlock();
			self.fileHandle = std::move(other.fileHandle);
			self.filePath = std::move(other.filePath);
		}

		void CreateOrOpenFileHandle(this FileLock& self, const std::filesystem::path& path)
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

		void DoLock(this FileLock& self) 
		{
			if (not self.HandleIsValid())
				throw Error::Boring32Error("File handle is null.");

			Win32::OVERLAPPED overlapped{};
			constexpr auto options = Win32::LockFileFlags::Exclusive | Win32::LockFileFlags::FailImmediately;
			// Requires generic read or generic write
			auto succeeded = Win32::LockFileEx(
				self.fileHandle.get(),
				options,
				0,
				0,
				0,
				&overlapped
			);
			if (not succeeded)
			{
				const auto lastError = Win32::GetLastError();
				throw Error::Win32Error(lastError, std::format("Failed to lock file {}.", self.filePath.string()));
			}
		}

		void DoUnlock(this FileLock& self)
		{
			if (not self.HandleIsValid())
				return;
			Win32::OVERLAPPED overlapped{};
			auto succeeded = Win32::UnlockFileEx(
				self.fileHandle.get(),
				0,
				0,
				0,
				&overlapped
			);
			if (succeeded)
				return;
			if (const auto lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::NotLocked)
				throw Error::Win32Error(lastError, std::format("Failed to unlock file {}.", self.filePath.string()));
		}

		void OpenHandleAndLock(this FileLock& self)
		{
			self.CreateOrOpenFileHandle(self.filePath);
			self.DoLock();
		}
	};

	static_assert(Concepts::BasicLockable<FileLock>);
}
