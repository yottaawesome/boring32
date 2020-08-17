#include "pch.hpp"
#include <stdexcept>
#include "include/Async/OverlappedOp.hpp"

namespace Boring32::Async
{
	OverlappedOp::~OverlappedOp()
	{ }

	OverlappedOp::OverlappedOp()
		: IoEvent(false, true, true, L""),
		IoOverlapped{},
		CallReturnValue(false),
		LastErrorValue(0)
	{
		IoOverlapped.hEvent = IoEvent.GetHandle();
	}

	OverlappedOp::OverlappedOp(
		const bool isInheritable,
		const bool manualReset,
		const bool isSignaled,
		const std::wstring name
	)
		: IoEvent(isInheritable, manualReset, isSignaled, name),
		IoOverlapped{},
		CallReturnValue(false),
		LastErrorValue(0)
	{ }

	OverlappedOp::OverlappedOp(const OverlappedOp& other)
		: IoOverlapped{}
	{
		Copy(other);
	}

	void OverlappedOp::operator=(const OverlappedOp& other)
	{
		Copy(other);
	}

	OverlappedOp::OverlappedOp(OverlappedOp&& other) noexcept
		: IoOverlapped{}
	{
		Move(other);
	}

	void OverlappedOp::operator=(OverlappedOp&& other) noexcept
	{
		Move(other);
	}

	void OverlappedOp::Copy(const OverlappedOp& other)
	{
		IoEvent = other.IoEvent;
		IoOverlapped = other.IoOverlapped;
		IoOverlapped.hEvent = IoEvent.GetHandle();
		CallReturnValue = other.CallReturnValue;
		LastErrorValue = other.LastErrorValue;
	}

	void OverlappedOp::Move(OverlappedOp& other) noexcept
	{
		IoEvent = std::move(other.IoEvent);
		IoOverlapped = other.IoOverlapped;
		IoOverlapped.hEvent = IoEvent.GetHandle();
		CallReturnValue = other.CallReturnValue;
		LastErrorValue = other.LastErrorValue;
	}
}