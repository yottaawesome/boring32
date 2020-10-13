#include "pch.hpp"
#include "include/Error/Win32Error.hpp"
#include "include/Async/OverlappedIo.hpp"

namespace Boring32::Async
{
	OverlappedIo::~OverlappedIo() { }

	OverlappedIo::OverlappedIo()
	{ }

	OverlappedIo::OverlappedIo(OverlappedIo&& other) noexcept
	:	OverlappedOp(std::move(other))
	{
		Move(other);
	}

	OverlappedIo& OverlappedIo::operator=(OverlappedIo&& other) noexcept
	{
		OverlappedOp::Move(other);
		Move(other);
		return *this;
	}

	void OverlappedIo::Move(OverlappedIo& other) noexcept
	{
		IoBuffer = std::move(other.IoBuffer);
	}

	void OverlappedIo::ResizeBuffer()
	{
		if (IsSuccessful() == false)
			throw std::runtime_error("ResizeBuffer(): operation is not successful");
		const uint64_t bytesTransferred = GetBytesTransferred();
		if (bytesTransferred > 0)
			IoBuffer.resize(bytesTransferred / sizeof(wchar_t));
	}
}