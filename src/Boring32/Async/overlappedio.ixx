export module boring32:async.overlappedio;
import std;
import :win32;
import :error;
import :async.overlappedop;

export namespace Boring32::Async
{
	struct OverlappedIo final : OverlappedOp
	{
		OverlappedIo() = default;
		// Non-copyable
		OverlappedIo(const OverlappedIo& other) = delete;
		auto operator=(const OverlappedIo& other) -> OverlappedIo& = delete;
		
		OverlappedIo(OverlappedIo&& other) noexcept
		{
			Move(other);
		}

		auto operator=(OverlappedIo&& other) noexcept -> OverlappedIo&
		{
			Move(other);
			return *this;
		}

		std::vector<std::byte> IoBuffer;

		private:
		auto Move(OverlappedIo& other) noexcept -> void
		{
			OverlappedOp::Move(other);
			IoBuffer = std::move(other.IoBuffer);
		}

		auto OnSuccess() -> void override
		{
			ResizeBuffer();
		}

		auto ResizeBuffer() -> void
		{
			if (not IsSuccessful())
				throw Error::Boring32Error("Operation is not successful");
			const std::uint64_t bytesTransferred = GetBytesTransferred();
			if (bytesTransferred > 0)
				IoBuffer.resize(bytesTransferred);
		}
	};
}
