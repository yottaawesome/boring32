module;

#include <vector>
#include <stdexcept>

module boring32.async:overlappedio;
import boring32.error;

namespace Boring32::Async
{
	OverlappedIo::~OverlappedIo() { }

	OverlappedIo::OverlappedIo()
	{ }

	OverlappedIo::OverlappedIo(OverlappedIo&& other) noexcept
	{
		Move(other);
	}

	OverlappedIo& OverlappedIo::operator=(OverlappedIo&& other) noexcept
	{
		Move(other);
		return *this;
	}

	void OverlappedIo::Move(OverlappedIo& other) noexcept
	{
		OverlappedOp::Move(other);
		IoBuffer = std::move(other.IoBuffer);
	}

	void OverlappedIo::ResizeBuffer()
	{
		if (IsSuccessful() == false)
			throw std::runtime_error("ResizeBuffer(): operation is not successful");
		const uint64_t bytesTransferred = GetBytesTransferred();
		if (bytesTransferred > 0)
			IoBuffer.resize(bytesTransferred);
	}

	void OverlappedIo::OnSuccess()
	{
		ResizeBuffer();
	}
}