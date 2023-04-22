export module boring32.async:overlappedio;
import :overlappedop;
import <vector>;

export namespace Boring32::Async
{
	class OverlappedIo : public OverlappedOp
	{
		public:
			virtual ~OverlappedIo() = default;
			OverlappedIo() = default;
			OverlappedIo(OverlappedIo&& other) noexcept;
			virtual OverlappedIo& operator=(OverlappedIo&& other) noexcept;

			// Non-copyable
		public:
			OverlappedIo(const OverlappedIo& other) = delete;
			virtual OverlappedIo& operator=(const OverlappedIo& other) = delete;

		public:
			std::vector<std::byte> IoBuffer;

		protected:
			virtual void Move(OverlappedIo& other) noexcept;
			virtual void OnSuccess() override;
			virtual void ResizeBuffer();
	};
}
