module;

#include <stdint.h>

export module boring32.async:overlappedop;
import std;
import :event;
import <win32.hpp>;
import boring32.error;

export namespace Boring32::Async
{
	class 
		[[nodiscard("This object must remain live while the I/O operation is in progress")]] 
		OverlappedOp
	{
		public:
			virtual ~OverlappedOp() = default;
			OverlappedOp()
			{
				m_ioOverlapped->hEvent = m_ioEvent.GetHandle();
			}
			
		// Shareable, moveable
		public:
			OverlappedOp(const OverlappedOp& other)
			{
				Share(other);
			}

			virtual OverlappedOp& operator=(const OverlappedOp& other)
			{
				Share(other);
				return *this;
			}

			OverlappedOp(OverlappedOp&& other) noexcept
				: m_ioOverlapped(nullptr)
			{
				Move(other);
			}

			virtual OverlappedOp& operator=(OverlappedOp&& other) noexcept
			{
				Move(other);
				return *this;
			}

		public:
			virtual bool WaitForCompletion(const DWORD timeout)
			{
				if (m_ioOverlapped == nullptr)
					throw Error::Boring32Error("IoOverlapped is null");
				bool successfulWait = m_ioEvent.WaitOnEvent(timeout, true);
				if (successfulWait)
					OnSuccess();
				return successfulWait;
			}

			virtual HANDLE GetWaitableHandle() const noexcept
			{
				return m_ioEvent.GetHandle();
			}

			virtual OVERLAPPED* GetOverlapped()
			{
				if (m_ioOverlapped == nullptr)
					throw Error::Boring32Error("IoOverlapped is null");
				return m_ioOverlapped.get();
			}

			virtual uint64_t GetStatus() const
			{
				if (m_ioOverlapped == nullptr)
					throw Error::Boring32Error("IoOverlapped is null");
				//STATUS_PENDING,
				//ERROR_IO_INCOMPLETE
				return m_lastError == ERROR_IO_PENDING ? m_ioOverlapped->Internal : m_lastError;
			}

			virtual uint64_t GetBytesTransferred() const noexcept
			{
				if (m_ioOverlapped == nullptr)
					return 0;
				return m_ioOverlapped->InternalHigh;
			}

			virtual bool IsReady() const
			{
				return m_ioOverlapped != nullptr;
			}

			virtual bool IsComplete() const
			{
				if (m_ioOverlapped == nullptr)
					return false;
				return m_ioOverlapped->Internal != STATUS_PENDING;
			}

			virtual bool IsSuccessful() const
			{
				if (m_ioOverlapped == nullptr)
					return false;
				// If the buffer is insufficient, Internal will be value 0x80000005L,
				// which is decimal value 2147483653. See error code STATUS_BUFFER_OVERFLOW:
				// https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
				return m_ioOverlapped->Internal == NOERROR;
			}

			virtual bool IsPartial() const
			{
				if (m_ioOverlapped == nullptr)
					return false;
				return m_ioOverlapped->Internal == 0x80000005L;// STATUS_BUFFER_OVERFLOW;
			}

			virtual void SetEvent(const bool signaled)
			{
				if (signaled)
					m_ioEvent.Signal();
				else
					m_ioEvent.Reset();
			}

			virtual DWORD LastError() const
			{
				return m_lastError;
			}

			virtual void LastError(const DWORD lastError)
			{
				m_lastError = lastError;
			}

		protected:
			virtual void Move(OverlappedOp& other) noexcept
			{
				m_ioEvent = std::move(other.m_ioEvent);
				m_ioOverlapped = std::move(other.m_ioOverlapped);
				m_lastError = other.m_lastError;
			}

			virtual void Share(const OverlappedOp& other)
			{
				m_ioEvent = other.m_ioEvent;
				m_ioOverlapped = other.m_ioOverlapped;
				m_lastError = other.m_lastError;
			}

			virtual void OnSuccess() {}

		protected:
			Event m_ioEvent{ false, true, false, L"" };
			std::shared_ptr<OVERLAPPED> m_ioOverlapped = std::make_shared<OVERLAPPED>();
			DWORD m_lastError = 0;
	};
}
