export module boring32:async_overlappedio;
import boring32.shared;
import :error;
import :async_overlappedop;

export namespace Boring32::Async
{
	struct OverlappedIo : OverlappedOp
	{
		virtual ~OverlappedIo() = default;
		OverlappedIo() = default;
		OverlappedIo(OverlappedIo&& other) noexcept
		{
			Move(other);
		}

		virtual OverlappedIo& operator=(OverlappedIo&& other) noexcept
		{
			Move(other);
			return *this;
		}

			// Non-copyable
		OverlappedIo(const OverlappedIo& other) = delete;
		virtual OverlappedIo& operator=(const OverlappedIo& other) = delete;

		std::vector<std::byte> IoBuffer;

		protected:
		virtual void Move(OverlappedIo& other) noexcept
		{
			OverlappedOp::Move(other);
			IoBuffer = std::move(other.IoBuffer);
		}

		virtual void OnSuccess() override
		{
			ResizeBuffer();
		}

		virtual void ResizeBuffer()
		{
			if (IsSuccessful() == false)
				throw Error::Boring32Error("Operation is not successful");
			const uint64_t bytesTransferred = GetBytesTransferred();
			if (bytesTransferred > 0)
				IoBuffer.resize(bytesTransferred);
		}
	};
}
