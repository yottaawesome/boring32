//#include <winternl.h>
//#include <ntstatus.h>
module boring32.async:overlappedop;
import boring32.error;
import <stdexcept>;
import <memory>;

namespace Boring32::Async
{
	OverlappedOp::~OverlappedOp() { }

	OverlappedOp::OverlappedOp()
	:	m_ioOverlapped(std::make_shared<OVERLAPPED>()),
		m_ioEvent(false, true, false, L""),
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
	:	m_ioOverlapped(nullptr)
	{
		Move(other);
	}

	OverlappedOp& OverlappedOp::operator=(OverlappedOp&& other) noexcept
	{
		Move(other);
		return *this;
	}

	bool OverlappedOp::WaitForCompletion(const DWORD timeout)
	{
		if (m_ioOverlapped == nullptr)
			throw Error::Boring32Error("IoOverlapped is null");
		bool successfulWait = m_ioEvent.WaitOnEvent(timeout, true);
		if (successfulWait)
			OnSuccess();
		return successfulWait;
	}

	HANDLE OverlappedOp::GetWaitableHandle() const
	{
		return m_ioEvent.GetHandle();
	}

	OVERLAPPED* OverlappedOp::GetOverlapped()
	{
		if (m_ioOverlapped == nullptr)
			throw Error::Boring32Error("IoOverlapped is null");
		return m_ioOverlapped.get();
	}

	uint64_t OverlappedOp::GetStatus() const
	{
		if (m_ioOverlapped == nullptr)
			throw Error::Boring32Error("IoOverlapped is null");
		//STATUS_PENDING,
		//ERROR_IO_INCOMPLETE
		return m_lastError == ERROR_IO_PENDING ? m_ioOverlapped->Internal : m_lastError;
	}

	uint64_t OverlappedOp::GetBytesTransferred() const
	{
		if (m_ioOverlapped == nullptr)
			return 0;
		return m_ioOverlapped->InternalHigh;
	}

	bool OverlappedOp::IsReady() const
	{
		return m_ioOverlapped != nullptr;
	}

	bool OverlappedOp::IsComplete() const
	{
		if (m_ioOverlapped == nullptr)
			return false;
		return m_ioOverlapped->Internal != STATUS_PENDING;
	}

	bool OverlappedOp::IsSuccessful() const
	{
		if (m_ioOverlapped == nullptr)
			return false;
		// If the buffer is insufficient, Internal will be value 0x80000005L,
		// which is decimal value 2147483653. See error code STATUS_BUFFER_OVERFLOW:
		// https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
		return m_ioOverlapped->Internal == NOERROR;
	}

	bool OverlappedOp::IsPartial() const
	{
		if (m_ioOverlapped == nullptr)
			return false;
		return m_ioOverlapped->Internal == 0x80000005L;// STATUS_BUFFER_OVERFLOW;
	}

	void OverlappedOp::SetEvent(const bool signaled)
	{
		if (signaled)
			m_ioEvent.Signal();
		else
			m_ioEvent.Reset();
	}

	DWORD OverlappedOp::LastError() const
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
		m_ioOverlapped = std::move(other.m_ioOverlapped);
		m_lastError = other.m_lastError;
	}

	void OverlappedOp::Share(const OverlappedOp& other)
	{
		m_ioEvent = other.m_ioEvent;
		m_ioOverlapped = other.m_ioOverlapped;
		m_lastError = other.m_lastError;
	}

	void OverlappedOp::OnSuccess() { }
}
