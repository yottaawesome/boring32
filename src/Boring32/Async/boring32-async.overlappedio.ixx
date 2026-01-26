export module boring32:async.overlappedio;
import std;
import boring32.win32;
import :error;
import :async.overlappedop;

export namespace Boring32::Async
{
	struct OverlappedIo final : OverlappedOp
	{
		OverlappedIo() = default;
		// Non-copyable
		OverlappedIo(const OverlappedIo& other) = delete;
		OverlappedIo& operator=(const OverlappedIo& other) = delete;
		
		OverlappedIo(OverlappedIo&& other) noexcept
		{
			Move(other);
		}

		OverlappedIo& operator=(OverlappedIo&& other) noexcept
		{
			Move(other);
			return *this;
		}

		std::vector<std::byte> IoBuffer;

		private:
		void Move(OverlappedIo& other) noexcept
		{
			OverlappedOp::Move(other);
			IoBuffer = std::move(other.IoBuffer);
		}

		void OnSuccess() override
		{
			ResizeBuffer();
		}

		void ResizeBuffer()
		{
			if (not IsSuccessful())
				throw Error::Boring32Error("Operation is not successful");
			const std::uint64_t bytesTransferred = GetBytesTransferred();
			if (bytesTransferred > 0)
				IoBuffer.resize(bytesTransferred);
		}
	};
}
