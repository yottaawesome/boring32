export module boring32:async.filerangelock;
import std;
import :win32;
import :concepts;
import :error;
import :raii;
import :util;
import :strings;

export namespace Boring32::Async
{
	class FileRangeLock final
	{
	public:
		~FileRangeLock() { unlock(); }
		constexpr FileRangeLock() = default;
		FileRangeLock(const FileRangeLock&) = delete;
		auto operator=(const FileRangeLock&) -> FileRangeLock& = delete;

		FileRangeLock(FileRangeLock&& other)
		{
			Move(other);
		}

		auto operator=(this FileRangeLock& self, FileRangeLock&& other) -> FileRangeLock&
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

		void DoLock(this FileRangeLock& self)
		{
			if (not self.HandleIsValid())
				throw Error::Boring32Error("File handle is null.");

			auto overlapped = Win32::OVERLAPPED{
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
				throw Error::Win32Error{ Win32::GetLastError(), std::format("Failed to lock file.")};
		}

		void DoUnlock(this FileRangeLock& self)
		{
			if (not self.HandleIsValid())
				return;
			auto overlapped = Win32::OVERLAPPED{
				.Offset = self.params.Offset,
				.OffsetHigh = self.params.OffsetHigh
			};
			const auto [lockSizeLow, lockSizeHigh] = Util::Decompose(self.params.Range);
			auto succeeded = 
				Win32::UnlockFileEx(
					self.fileHandle,
					0,
					lockSizeLow,
					lockSizeHigh,
					&overlapped
				);
			if (not succeeded)
				if (const auto lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::NotLocked)
					throw Error::Win32Error{lastError, std::format("Failed to unlock file.")};
		}

		constexpr auto HandleIsValid(this const FileRangeLock& self) -> bool
		{
			return self.fileHandle != Win32::InvalidHandleValue and self.fileHandle;
		}

	private:
		Win32::HANDLE fileHandle = nullptr;
		LockParams params;

		void Move(this FileRangeLock& self, FileRangeLock& other)
		{
			self.DoUnlock();
			self.fileHandle = std::exchange(other.fileHandle, nullptr);
			self.params = other.params;
		}
	};
	static_assert(Concepts::Lockable<FileRangeLock>);
}
