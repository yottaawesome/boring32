#include "pch.hpp"
#include <stdexcept>
#include "include/Async/OverlappedOp.hpp"
#include "include/Error/Win32Exception.hpp"

namespace Boring32::Async
{
	OverlappedOp::~OverlappedOp()
	{
		if (IoOverlapped)
		{
			delete IoOverlapped;
			IoOverlapped = nullptr;
		}
	}

	OverlappedOp::OverlappedOp()
		: IoEvent(false, true, false, L""),
		IoOverlapped{},
		CallReturnValue(false),
		LastErrorValue(0)
	{
		IoOverlapped = new OVERLAPPED({});
		IoOverlapped->hEvent = IoEvent.GetHandle();
	}

	OverlappedOp::OverlappedOp(
		const bool isInheritable,
		const std::wstring name
	)
		: IoEvent(isInheritable, true, false, name),
		IoOverlapped{},
		CallReturnValue(false),
		LastErrorValue(0)
	{
		IoOverlapped = new OVERLAPPED({});
		IoOverlapped->hEvent = IoEvent.GetHandle();
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

	void OverlappedOp::WaitForCompletion(const DWORD timeout)
	{
		if (IoOverlapped == nullptr)
			throw std::runtime_error("IoOverlapped is null");
		IoEvent.WaitOnEvent(timeout);
	}

	OVERLAPPED* OverlappedOp::GetOverlapped()
	{
		if (IoOverlapped == nullptr)
			throw std::runtime_error("IoOverlapped is null");
		return IoOverlapped;
	}

	uint64_t OverlappedOp::GetStatus()
	{
		if (IoOverlapped == nullptr)
			throw std::runtime_error("IoOverlapped is null");
		//STATUS_PENDING,
		//ERROR_IO_INCOMPLETE
		return IoOverlapped->Internal;
	}

	uint64_t OverlappedOp::GetBytesTransferred()
	{
		if (IoOverlapped == nullptr)
			throw std::runtime_error("IoOverlapped is null");
		return IoOverlapped->InternalHigh;
	}

	bool OverlappedOp::IsComplete()
	{
		if (IoOverlapped == nullptr)
			throw std::runtime_error("IoOverlapped is null");
		return IoOverlapped->Internal != STATUS_PENDING;
	}

	bool OverlappedOp::IsSuccessful()
	{
		if (IoOverlapped == nullptr)
			throw std::runtime_error("IoOverlapped is null");
		return IoOverlapped->Internal == NOERROR;
	}

	void OverlappedOp::Cancel()
	{
		if (IoHandle == nullptr)
			throw std::runtime_error("No IoHandle to cancel on");
		if (IoOverlapped == nullptr)
			throw std::runtime_error("IoOverlapped is null");
		if (CancelIo(IoHandle.GetHandle()) == false)
			throw Error::Win32Exception("CancelIo failed", GetLastError());
	}

	bool OverlappedOp::Cancel(std::nothrow_t)
	{
		if (IoHandle == nullptr)
			return false;
		if (IoOverlapped == nullptr)
			throw std::runtime_error("IoOverlapped is null");
		return CancelIo(IoHandle.GetHandle());
	}

	void OverlappedOp::Move(OverlappedOp& other) noexcept
	{
		IoEvent = std::move(other.IoEvent);
		IoHandle = std::move(other.IoHandle);
		IoOverlapped = other.IoOverlapped;
		other.IoOverlapped = nullptr;
		IoOverlapped->hEvent = IoEvent.GetHandle();
		CallReturnValue = other.CallReturnValue;
		LastErrorValue = other.LastErrorValue;
	}
}