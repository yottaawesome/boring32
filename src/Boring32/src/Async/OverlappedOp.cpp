#include "pch.hpp"
#include <stdexcept>
#include "include/Async/OverlappedOp.hpp"
#include "include/Error/Win32Exception.hpp"

namespace Boring32::Async
{
	OverlappedOp::~OverlappedOp()
	{
		if (m_ioOverlapped)
		{
			delete m_ioOverlapped;
			m_ioOverlapped = nullptr;
		}
	}

	OverlappedOp::OverlappedOp()
	:	m_ioOverlapped(nullptr),
		CallReturnValue(false),
		LastErrorValue(0)
	{ }

	OverlappedOp::OverlappedOp(const Raii::Win32Handle& handle)
	:	m_ioHandle(handle),
		m_ioEvent(false, true, false, L""),
		m_ioOverlapped{},
		CallReturnValue(false),
		LastErrorValue(0)
	{
		m_ioOverlapped = new OVERLAPPED({});
		m_ioOverlapped->hEvent = m_ioEvent.GetHandle();
	}

	OverlappedOp::OverlappedOp(OverlappedOp&& other) noexcept
		: m_ioOverlapped{}
	{
		Move(other);
	}

	void OverlappedOp::operator=(OverlappedOp&& other) noexcept
	{
		Move(other);
	}

	void OverlappedOp::WaitForCompletion(const DWORD timeout)
	{
		if (m_ioOverlapped == nullptr)
			throw std::runtime_error("IoOverlapped is null");
		m_ioEvent.WaitOnEvent(timeout);
	}

	OVERLAPPED* OverlappedOp::GetOverlapped()
	{
		if (m_ioOverlapped == nullptr)
			throw std::runtime_error("IoOverlapped is null");
		return m_ioOverlapped;
	}

	uint64_t OverlappedOp::GetStatus()
	{
		if (m_ioOverlapped == nullptr)
			throw std::runtime_error("IoOverlapped is null");
		//STATUS_PENDING,
		//ERROR_IO_INCOMPLETE
		return m_ioOverlapped->Internal;
	}

	uint64_t OverlappedOp::GetBytesTransferred()
	{
		if (m_ioOverlapped == nullptr)
			throw std::runtime_error("IoOverlapped is null");
		return m_ioOverlapped->InternalHigh;
	}

	bool OverlappedOp::IsComplete()
	{
		if (m_ioOverlapped == nullptr)
			throw std::runtime_error("IoOverlapped is null");
		return m_ioOverlapped->Internal != STATUS_PENDING;
	}

	bool OverlappedOp::IsSuccessful()
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
			throw Error::Win32Exception("CancelIo failed", GetLastError());
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
		m_ioEvent = std::move(other.m_ioEvent);
		m_ioHandle = std::move(other.m_ioHandle);
		m_ioOverlapped = other.m_ioOverlapped;
		other.m_ioOverlapped = nullptr;
		m_ioOverlapped->hEvent = m_ioEvent.GetHandle();
		CallReturnValue = other.CallReturnValue;
		LastErrorValue = other.LastErrorValue;
	}
}