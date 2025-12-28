export module boring32:async_filerangelock;
import std;
import boring32.win32;
import :concepts;
import :error;
import :raii;
import :util;
import :strings;

export namespace Boring32::Async
{
	struct FileRangeLock final
	{
		~FileRangeLock() { unlock(); }
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

		struct LockParams
		{
			bool Acquire = true;
			bool FailImmediately = true;
			std::uint64_t Range = std::numeric_limits<std::uint64_t>::max();
			Win32::DWORD Offset = 0;
			Win32::DWORD OffsetHigh = 0;
		};

		FileRangeLock(Win32::HANDLE handle, LockParams params)
			: fileHandle(handle), params(params)
		{
			if (not HandleIsValid())
				throw Error::Boring32Error("File handle is not a valid file handle.");
			params.Acquire ? DoLock() : void();
		}

		void lock(this FileRangeLock& self)
		{ 
			self.DoLock(); 
		}
		
		void unlock(this FileRangeLock& self) noexcept
		try
		{
			self.DoUnlock(); 
		}
		catch (const std::exception& ex)
		{
			std::wcerr << std::format(L"Fatal exception in FileRangeLock::unlock(): {}.", Strings::ConvertString(ex.what()));
			std::terminate();
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
			return self.fileHandle != Win32::InvalidHandleValue and self.fileHandle;
		}

		void DoLock(this FileRangeLock& self)
		{
			if (not self.HandleIsValid())
				throw Error::Boring32Error("File handle is null.");

			Win32::OVERLAPPED overlapped{
				.Offset = self.params.Offset,
				.OffsetHigh = self.params.OffsetHigh
			};
			auto options = Win32::LockFileFlags::Exclusive |
				(self.params.FailImmediately ? Win32::LockFileFlags::FailImmediately : 0);
			// Requires generic read or generic write
			const auto [lockSizeLow, lockSizeHigh] = Util::Decompose(self.params.Range);
			auto succeeded = Win32::LockFileEx(
				self.fileHandle,
				options,
				0,
				lockSizeLow,
				lockSizeHigh,
				&overlapped
			);
			if (not succeeded)
			{
				const auto lastError = Win32::GetLastError();
				throw Error::Win32Error(lastError, std::format("Failed to lock file."));
			}
		}

		void DoUnlock(this FileRangeLock& self)
		{
			if (not self.HandleIsValid())
				return;
			Win32::OVERLAPPED overlapped{
				.Offset = self.params.Offset,
				.OffsetHigh = self.params.OffsetHigh
			};
			const auto [lockSizeLow, lockSizeHigh] = Util::Decompose(self.params.Range);
			auto succeeded = Win32::UnlockFileEx(
				self.fileHandle,
				0,
				lockSizeLow,
				lockSizeHigh,
				&overlapped
			);
			if (not succeeded)
				if (const auto lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::NotLocked)
					throw Error::Win32Error(lastError, std::format("Failed to unlock file."));
		}

	private:
		Win32::HANDLE fileHandle;
		LockParams params;

		void Move(this FileRangeLock& self, FileRangeLock& other)
		{
			self.DoUnlock();
			self.fileHandle = std::move(other.fileHandle);
			self.params = other.params;
			other.fileHandle = nullptr;
		}
	};
	static_assert(Concepts::Lockable<FileRangeLock>);
}
