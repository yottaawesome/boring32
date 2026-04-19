export module boring32:async.overlappedop;
import std;
import :win32;
import :error;
import :async.event;

export namespace Boring32::Async
{
	struct [[nodiscard("This object must remain live while the I/O operation is in progress")]] 
	OverlappedOp
	{
		virtual ~OverlappedOp() = default;
		OverlappedOp()
		{
			m_ioOverlapped->hEvent = m_ioEvent.GetHandle();
		}
			
		// Shareable, moveable
		OverlappedOp(const OverlappedOp& other) = delete;
		auto operator=(const OverlappedOp& other) -> OverlappedOp& = delete;

		OverlappedOp(OverlappedOp&& other) noexcept
		{
			Move(other);
		}

		virtual auto operator=(OverlappedOp&& other) noexcept -> OverlappedOp&
		{
			Move(other);
			return *this;
		}

		virtual auto WaitForCompletion(const Win32::DWORD timeout) -> bool
		{
			if (m_ioOverlapped == nullptr)
				throw Error::Boring32Error("IoOverlapped is null");
			bool successfulWait = m_ioEvent.WaitOnEvent(timeout, true);
			if (successfulWait)
				OnSuccess();
			return successfulWait;
		}

		virtual auto GetWaitableHandle() const noexcept -> Win32::HANDLE
		{
			return m_ioEvent.GetHandle();
		}

		virtual auto GetOverlapped() -> Win32::OVERLAPPED*
		{
			if (m_ioOverlapped == nullptr)
				throw Error::Boring32Error("IoOverlapped is null");
			return m_ioOverlapped.get();
		}

		virtual auto GetStatus() const -> std::uint64_t
		{
			if (m_ioOverlapped == nullptr)
				throw Error::Boring32Error("IoOverlapped is null");
			//STATUS_PENDING,
			//ERROR_IO_INCOMPLETE
			return m_lastError == Win32::ErrorCodes::IoPending ? m_ioOverlapped->Internal : m_lastError;
		}

		virtual auto GetBytesTransferred() const noexcept -> std::uint64_t
		{
			if (m_ioOverlapped == nullptr)
				return 0;
			return m_ioOverlapped->InternalHigh;
		}

		virtual auto IsReady() const -> bool
		{
			return m_ioOverlapped != nullptr;
		}

		virtual auto IsComplete() const -> bool
		{
			if (m_ioOverlapped == nullptr)
				return false;
			return m_ioOverlapped->Internal != Win32::NTStatus::Pending;
		}

		virtual auto IsSuccessful() const -> bool
		{
			if (m_ioOverlapped == nullptr)
				return false;
			// If the buffer is insufficient, Internal will be value 0x80000005L,
			// which is decimal value 2147483653. See error code STATUS_BUFFER_OVERFLOW:
			// https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
			return m_ioOverlapped->Internal == Win32::ErrorCodes::NoError;
		}

		virtual auto IsPartial() const -> bool
		{
			if (m_ioOverlapped == nullptr)
				return false;
			return m_ioOverlapped->Internal == 0x80000005L;// STATUS_BUFFER_OVERFLOW;
		}

		virtual auto SetEvent(const bool signaled) -> void
		{
			if (signaled)
				m_ioEvent.Signal();
			else
				m_ioEvent.Reset();
		}

		virtual auto LastError() const -> Win32::DWORD
		{
			return m_lastError;
		}

		virtual auto LastError(const Win32::DWORD lastError) -> void
		{
			m_lastError = lastError;
		}

	protected:
		virtual auto Move(OverlappedOp& other) noexcept -> void
		{
			m_ioEvent = std::move(other.m_ioEvent);
			m_ioOverlapped = std::move(other.m_ioOverlapped);
			m_lastError = other.m_lastError;
		}

		virtual auto OnSuccess() -> void {}

		ManualResetEvent m_ioEvent{ false, false };
		std::shared_ptr<Win32::OVERLAPPED> m_ioOverlapped = std::make_shared<Win32::OVERLAPPED>();
		Win32::DWORD m_lastError = 0;
	};
}
