#include "pch.hpp"
#include <stdexcept>
#include "include/Async/OverlappedOp.hpp"
#include "include/Error/Win32Error.hpp"

namespace Boring32::Async
{
	OverlappedOp::~OverlappedOp() { }

	OverlappedOp::OverlappedOp()
	:	m_ioOverlapped(nullptr),
		m_ioEvent(false, true, false, L""),
		m_lastError(0)
	{ }

	OverlappedOp::OverlappedOp(const Raii::Win32Handle& handle)
	:	m_ioHandle(handle),
		m_ioEvent(false, true, false, L""),
		m_ioOverlapped(std::make_shared<OVERLAPPED>()),
		m_lastError(0)
	{
		m_ioOverlapped->hEvent = m_ioEvent.GetHandle();
	}

	OverlappedOp::OverlappedOp(const OverlappedOp& other)
	{
		Share(other);
	}

	OverlappedOp& OverlappedOp::operator=(const OverlappedOp& other)
	{
		Share(other);
		return *this;
	}

	OverlappedOp::OverlappedOp(OverlappedOp&& other) noexcept
		: m_ioOverlapped(nullptr)
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
		m_ioEvent.WaitOnEvent(timeout, true);
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
		return m_lastError == ERROR_IO_PENDING ? m_ioOverlapped->Internal : m_lastError;
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

		// If the buffer is insufficient, Internal will be value 0x80000005L,
		// which is decimal value 2147483653. See error code STATUS_BUFFER_OVERFLOW:
		// https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
		return m_ioOverlapped->Internal == NOERROR;
	}

	void OverlappedOp::SetEvent(const bool signaled)
	{
		if (signaled)
			m_ioEvent.Signal();
		else
			m_ioEvent.Reset();
	}

	void OverlappedOp::Cancel()
	{
		if (m_ioHandle == nullptr)
			throw std::runtime_error("No IoHandle to cancel on");
		if (m_ioOverlapped == nullptr)
			throw std::runtime_error("IoOverlapped is null");
		if (CancelIo(m_ioHandle.GetHandle()) == false)
			throw Error::Win32Error("CancelIo failed", GetLastError());
		m_ioHandle = nullptr;
	}

	bool OverlappedOp::Cancel(std::nothrow_t)
	{
		if (m_ioHandle == nullptr)
			return false;
		if (m_ioOverlapped == nullptr)
			throw std::runtime_error("IoOverlapped is null");
		const bool returnValue = CancelIo(m_ioHandle.GetHandle());
		m_ioHandle = nullptr;
		return returnValue;
	}

	DWORD OverlappedOp::LastError()
	{
		return m_lastError;
	}

	void OverlappedOp::LastError(const DWORD lastError)
	{
		m_lastError = lastError;
	}

	void OverlappedOp::Move(OverlappedOp& other) noexcept
	{
		m_ioEvent = std::move(other.m_ioEvent);
		m_ioHandle = std::move(other.m_ioHandle);
		m_ioOverlapped = std::move(other.m_ioOverlapped);
		m_lastError = other.m_lastError;
	}

	void OverlappedOp::Share(const OverlappedOp& other)
	{
		m_ioEvent = other.m_ioEvent;
		m_ioHandle = other.m_ioHandle;
		m_ioOverlapped = other.m_ioOverlapped;
		m_lastError = other.m_lastError;
	}
}
