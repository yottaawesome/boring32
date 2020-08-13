#include "pch.hpp"
#include <stdexcept>
#include "include/Async/OverlappedIo.hpp"

namespace Boring32::Async
{
	OverlappedIo::~OverlappedIo()
	{ }
	
	OverlappedIo::OverlappedIo()
	:	IoEvent(true, false, true, true, L""),
		IoOverlapped{},
		CallReturnValue(false),
		LastErrorValue(0)
	{
		IoOverlapped.hEvent = IoEvent.GetHandle();
	}

	OverlappedIo::OverlappedIo(
		const bool createOrOpen,
		const bool isInheritable,
		const bool manualReset,
		const bool isSignaled,
		const std::wstring name
	)
	:	IoEvent(createOrOpen, isInheritable, manualReset, isSignaled, name),
		IoOverlapped{},
		CallReturnValue(false),
		LastErrorValue(0)
	{ }

	OverlappedIo::OverlappedIo(const OverlappedIo& other)
	:	IoOverlapped{}
	{
		Copy(other);
	}

	void OverlappedIo::operator=(const OverlappedIo& other)
	{
		Copy(other);
	}

	OverlappedIo::OverlappedIo(OverlappedIo&& other) noexcept
	:	IoOverlapped{}
	{
		Move(other);
	}

	void OverlappedIo::operator=(OverlappedIo&& other) noexcept
	{
		Move(other);
	}

	void OverlappedIo::Copy(const OverlappedIo& other)
	{
		IoEvent = other.IoEvent;
		IoHandle = other.IoHandle;
		IoOverlapped = other.IoOverlapped;
		IoOverlapped.hEvent = IoEvent.GetHandle();
		CallReturnValue = other.CallReturnValue;
		LastErrorValue = other.LastErrorValue;
	}

	void OverlappedIo::Move(OverlappedIo& other) noexcept
	{
		IoEvent = std::move(other.IoEvent);
		IoHandle = std::move(other.IoHandle);
		IoOverlapped = other.IoOverlapped;
		IoOverlapped.hEvent = IoEvent.GetHandle();
		CallReturnValue = other.CallReturnValue;
		LastErrorValue = other.LastErrorValue;
	}

	DWORD OverlappedIo::GetBytesTransferred(const bool wait)
	{
		DWORD bytesTransferred = 0;
		bool succeeded = GetOverlappedResult(
			IoHandle.GetHandle(),
			&IoOverlapped,
			&bytesTransferred,
			wait
		);
		if (succeeded == false)
			throw std::runtime_error("Failed to get overlapped result");

		return bytesTransferred;
	}
}