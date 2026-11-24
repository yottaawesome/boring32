export module Boring32:Async.OverlappedIO;
import std;
import :Win32;
import :Error;
import :Async.OverlappedOp;

export namespace Boring32::Async
{
	class OverlappedIo final : public OverlappedOp
	{
	public:
		OverlappedIo() = default;
		// Non-copyable
		OverlappedIo(const OverlappedIo& other) = delete;
		auto operator=(const OverlappedIo& other) -> OverlappedIo& = delete;
		
		OverlappedIo(OverlappedIo&& other) noexcept = default;
		auto operator=(OverlappedIo&& other) noexcept -> OverlappedIo& = default;

		std::vector<std::byte> IoBuffer;

	private:
		void OnSuccess(this auto&& self)
		{
			self.ResizeBuffer();
		}

		void ResizeBuffer()
		{
			if (not IsSuccessful())
				throw Error::Boring32Error{ "Operation is not successful" };
			const std::uint64_t bytesTransferred = GetBytesTransferred();
			if (bytesTransferred > 0)
				IoBuffer.resize(bytesTransferred);
		}
	};
}
