#include "pch.hpp"
#include <stdexcept>
#include "include/Async/OverlappedOp.hpp"
#include "include/Error/Win32Error.hpp"

namespace Boring32::Async
{
	OverlappedOp::~OverlappedOp() { }

	OverlappedOp::OverlappedOp()
	:	m_ioOverlapped(nullptr),
		CallReturnValue(false),
		LastErrorValue(0)
	{ }

	OverlappedOp::OverlappedOp(const Raii::Win32Handle& handle)
	:	m_ioHandle(handle),
		m_ioEvent(false, true, false, L""),
		m_ioOverlapped(std::make_unique<OVERLAPPED>()),
		CallReturnValue(false),
		LastErrorValue(0)
	{
		m_ioOverlapped->hEvent = m_ioEvent.GetHandle();
	}

	OverlappedOp::OverlappedOp(OverlappedOp&& other) noexcept
		: m_ioOverlapped{}
	{
		Move(other);
	}

	OverlappedOp& OverlappedOp::operator=(OverlappedOp&& other) noexcept
	{
		Move(other);
		return *this;
	}

	void OverlappedOp::WaitForCompletion(const DWORD timeout)
	{
		if (m_ioOverlapped == nullptr)
			throw std::runtime_error("IoOverlapped is null");
		m_ioEvent.WaitOnEvent(timeout);
	}

	HANDLE OverlappedOp::GetWaitableHandle() const
	{
		return m_ioEvent.GetHandle();
	}

	OVERLAPPED* OverlappedOp::GetOverlapped()
	{
		if (m_ioOverlapped == nullptr)
			throw std::runtime_error("IoOverlapped is null");
		return m_ioOverlapped.get();
	}

	uint64_t OverlappedOp::GetStatus() const
	{
		if (m_ioOverlapped == nullptr)
			throw std::runtime_error("IoOverlapped is null");
		//STATUS_PENDING,
		//ERROR_IO_INCOMPLETE
		return m_ioOverlapped->Internal;
	}

	uint64_t OverlappedOp::GetBytesTransferred() const
	{
		if (m_ioOverlapped == nullptr)
			throw std::runtime_error("IoOverlapped is null");
		return m_ioOverlapped->InternalHigh;
	}

	bool OverlappedOp::IsComplete() const
	{
		if (m_ioOverlapped == nullptr)
			throw std::runtime_error("IoOverlapped is null");
		return m_ioOverlapped->Internal != STATUS_PENDING;
	}

	bool OverlappedOp::IsSuccessful() const
	{
		if (m_ioOverlapped == nullptr)
			throw std::runtime_error("IoOverlapped is null");
		return m_ioOverlapped->Internal == NOERROR;
	}

	void OverlappedOp::Cancel()
	{
		if (m_ioHandle == nullptr)
			throw std::runtime_error("No IoHandle to cancel on");
		if (m_ioOverlapped == nullptr)
			throw std::runtime_error("IoOverlapped is null");
		if (CancelIo(m_ioHandle.GetHandle()) == false)
			throw Error::Win32Error("CancelIo failed", GetLastError());
	}

	bool OverlappedOp::Cancel(std::nothrow_t)
	{
		if (m_ioHandle == nullptr)
			return false;
		if (m_ioOverlapped == nullptr)
			throw std::runtime_error("IoOverlapped is null");
		return CancelIo(m_ioHandle.GetHandle());
	}

	void OverlappedOp::Move(OverlappedOp& other) noexcept
	{
		CallReturnValue = other.CallReturnValue;
		LastErrorValue = other.LastErrorValue;
		m_ioEvent = std::move(other.m_ioEvent);
		m_ioHandle = std::move(other.m_ioHandle);
		m_ioOverlapped = std::move(other.m_ioOverlapped);
		if (m_ioOverlapped != nullptr)
			m_ioOverlapped->hEvent = m_ioEvent.GetHandle();
	}
}