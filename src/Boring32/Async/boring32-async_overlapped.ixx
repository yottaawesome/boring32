export module boring32:async_overlapped;
import std;
import boring32.win32;
import :error;
import :concepts;

export namespace Boring32::Async
{
	struct [[nodiscard("This must remain alive while the IO operation is in progress.")]] 
	Overlapped final : Win32::OVERLAPPED
	{
		~Overlapped() { Close(); }

		Overlapped(const Overlapped&) = delete;
		Overlapped& operator=(const Overlapped&) = delete;

		// TODO: OVERLAPPED is not movable either.
		// May need to add a Reset() method.
		Overlapped(Overlapped&& other) { swap(other); };
		Overlapped& operator=(Overlapped&& other) { swap(other); };

		Overlapped() : Win32::OVERLAPPED{}
		{
			hEvent = Win32::CreateEventW(nullptr, true, false, nullptr);
			if (not hEvent)
				throw Error::Win32Error(Win32::GetLastError(), "CreateEventW() failed");
		}

		bool Wait(Concepts::Duration auto waitTime, std::string_view msg)
		{
			while (not Wait(waitTime))
				std::println("{}", msg);
			return true;
		}

		bool Wait()
		{
			return Wait(std::chrono::milliseconds{ Win32::Infinite });
		}

		bool Wait(Concepts::Duration auto millis)
		{
			if (not hEvent)
				throw std::runtime_error("No event");

			auto count = std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(millis).count());
			return Win32::WaitForSingleObject(hEvent, static_cast<Win32::DWORD>(count)) == Win32::WaitObject0;
		}

		bool IsPartial() const noexcept
		{
			return this->Internal == 0x80000005L;// STATUS_BUFFER_OVERFLOW;
		}

		bool Pending() const noexcept
		{
			return Internal == 0x00000103L; // STATUS_IO_PENDING
		}

		std::uint64_t GetBytesRead() const noexcept
		{
			return this->InternalHigh;
		}

		void swap(Overlapped& other) noexcept
		{
			Win32::HANDLE temp = hEvent;
			hEvent = other.hEvent;
			other.hEvent = temp;
		}

		void Close() noexcept
		{
			if (hEvent)
			{
				Win32::CloseHandle(hEvent);
				hEvent = nullptr;
			}
		}

		void reset() noexcept
		{
			Close();
		}
	};
	static_assert(std::movable<Overlapped>);
	static_assert(not std::copyable<Overlapped>);
	static_assert(std::swappable<Overlapped>);

	struct AsyncRead final
	{
		~AsyncRead()
		{
			if (Operation.Pending())
				Win32::CancelIo(Pipe);
		}

		AsyncRead(Win32::HANDLE handle, Win32::DWORD bytesToRead)
			: Pipe(handle), BytesToRead(bytesToRead)
		{
			if (not handle)
				throw std::runtime_error("Handle cannot be nullptr");
			Start();
		}

		auto Ready() -> bool
		{
			return Operation.Wait();
		}

		auto FinishReading() -> void
		{
			Operation.Wait();
			if (not Operation.IsPartial())
			{
				Data.resize(Operation.GetBytesRead());
				return;
			}

			std::uint64_t totalBytesRead = Operation.GetBytesRead();
			while (true)
			{
				Operation = Async::Overlapped{};
				Win32::BOOL success = Win32::ReadFile(
					Pipe,
					Data.data() + totalBytesRead,
					BytesToRead,
					nullptr, // not used for async IO
					&Operation
				);
				Operation.Wait(std::chrono::milliseconds{ 1000 },"Waiting for read to complete.");
				totalBytesRead += Operation.GetBytesRead();
				if (success)
				{
					Data.resize(totalBytesRead);
					break;
				}
				if (Win32::DWORD lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::MoreData)
					throw Error::Win32Error(lastError, "ReadFile() failed.");
				Data.resize(Data.size() + BytesToRead);
			}
		}

		auto GetData() const& noexcept -> std::vector<std::byte>
		{
			return Data;
		}

		auto GetData() && noexcept -> std::vector<std::byte>&&
		{
			return std::move(Data);
		}

	private:
		auto Start() -> void
		{
			Data.resize(BytesToRead);
			Win32::BOOL success = Win32::ReadFile(
				Pipe,
				Data.data(),
				BytesToRead,
				nullptr, // not used for async IO
				&Operation
			);
			if (auto lastError = Win32::GetLastError(); not success and lastError != Win32::ErrorCodes::IoPending)
				throw Error::Win32Error(lastError, "IO operation failed");
		}

		std::vector<std::byte> Data;
		Overlapped Operation;
		Win32::HANDLE Pipe = nullptr;
		Win32::DWORD BytesToRead = 0;
	};
	static_assert(not std::movable<AsyncRead>);
	static_assert(not std::copyable<AsyncRead>);
}
