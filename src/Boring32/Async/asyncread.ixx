export module boring32:async.asyncread;
import std;
import :win32;
import :error;
import :async.overlapped;

export namespace Boring32::Async
{
	class AsyncRead final
	{
	public:
		~AsyncRead()
		{
			if (Operation.Pending())
				Win32::CancelIo(Pipe);
		}

		AsyncRead(Win32::HANDLE handle, Win32::DWORD bytesToRead)
			: Pipe(handle)
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
				auto success = Win32::ReadFile(
					Pipe,
					Data.data() + totalBytesRead,
					BytesToRead,
					nullptr, // not used for async IO
					&Operation
				);
				Operation.Wait(std::chrono::milliseconds{ 1000 }, "Waiting for read to complete.");
				totalBytesRead += Operation.GetBytesRead();
				if (success)
				{
					Data.resize(totalBytesRead);
					break;
				}
				if (auto lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::MoreData)
					throw Error::Win32Error{ lastError, "ReadFile() failed." };
				Data.resize(Data.size() + BytesToRead);
			}
		}

		auto GetData(this auto&& self) noexcept -> decltype(auto)
		{
			return std::forward_like<decltype(self)>(Data);
		}

	private:
		auto Start() -> void
		{
			Data.resize(BytesToRead);
			auto success = Win32::ReadFile(
				Pipe,
				Data.data(),
				BytesToRead,
				nullptr, // not used for async IO
				&Operation
			);
			if (not success)
				if (auto lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::IoPending)
					throw Error::Win32Error{ lastError, "IO operation failed" };
		}

		std::vector<std::byte> Data;
		Overlapped Operation;
		Win32::HANDLE Pipe = nullptr;
		Win32::DWORD BytesToRead = 0;
	};
	static_assert(not std::movable<AsyncRead>);
	static_assert(not std::copyable<AsyncRead>);
}