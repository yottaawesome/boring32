module;

#include <source_location>;

export module boring32.async:overlappedio;
import <vector>;
import <stdexcept>;
import boring32.error;
import :overlappedop;

export namespace Boring32::Async
{
	class OverlappedIo : public OverlappedOp
	{
		public:
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
		public:
			OverlappedIo(const OverlappedIo& other) = delete;
			virtual OverlappedIo& operator=(const OverlappedIo& other) = delete;

		public:
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
