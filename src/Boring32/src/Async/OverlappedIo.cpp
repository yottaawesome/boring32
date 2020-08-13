#include "pch.hpp"
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
		IoOverlapped = other.IoOverlapped;
		IoOverlapped.hEvent = IoEvent.GetHandle();
		CallReturnValue = other.CallReturnValue;
		LastErrorValue = other.LastErrorValue;

	}

	void OverlappedIo::Move(OverlappedIo& other) noexcept
	{
		IoEvent = std::move(other.IoEvent);
		IoOverlapped = other.IoOverlapped;
		IoOverlapped.hEvent = IoEvent.GetHandle();
		CallReturnValue = other.CallReturnValue;
		LastErrorValue = other.LastErrorValue;
	}
}