#include "pch.hpp"
#include "include/Error/Win32Exception.hpp"
#include "include/Async/OverlappedIo.hpp"

namespace Boring32::Async
{
	OverlappedIo::~OverlappedIo()
	{ }
	
	OverlappedIo::OverlappedIo()
	{
		IoOverlapped.hEvent = IoEvent.GetHandle();
	}

	OverlappedIo::OverlappedIo(
		const bool isInheritable,
		const bool manualReset,
		const bool isSignaled,
		const std::wstring name
	)
	:	OverlappedOp(isInheritable, manualReset, isSignaled, name)
	{ }

	OverlappedIo::OverlappedIo(const OverlappedIo& other)
	:	OverlappedOp(other)
	{
		Copy(other);
	}

	void OverlappedIo::operator=(const OverlappedIo& other)
	{
		Copy(other);
	}

	OverlappedIo::OverlappedIo(OverlappedIo&& other) noexcept
	:	OverlappedOp(other)
	{
		Move(other);
	}

	void OverlappedIo::operator=(OverlappedIo&& other) noexcept
	{
		Move(other);
	}

	void OverlappedIo::Copy(const OverlappedIo& other)
	{
		IoBuffer = other.IoBuffer;
		IoHandle = other.IoHandle;
	}

	void OverlappedIo::Move(OverlappedIo& other) noexcept
	{
		IoBuffer = std::move(other.IoBuffer);
		IoHandle = std::move(other.IoHandle);
	}

	bool OverlappedIo::GetBytesTransferred(const bool wait, DWORD& outBytes)
	{
		outBytes = 0;
		bool succeeded = GetOverlappedResult(
			IoHandle.GetHandle(),
			&IoOverlapped,
			&outBytes,
			wait
		);
		const DWORD lastError = GetLastError();
		if (succeeded == false && lastError != ERROR_IO_PENDING)
			throw Error::Win32Exception("Failed to get overlapped result", lastError);
		return succeeded;
	}
}