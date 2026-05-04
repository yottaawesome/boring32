export module boring32:async.overlappedop;
import std;
import :win32;
import :error;
import :async.event;

export namespace Boring32::Async
{
	class OverlappedOp [[nodiscard("This object must remain live while the I/O operation is in progress")]]
	{
	public:
		OverlappedOp()
		{
			m_ioOverlapped->hEvent = m_ioEvent.GetHandle();
		}
			
		// Not copyable
		OverlappedOp(const OverlappedOp& other) = delete;
		auto operator=(const OverlappedOp& other) -> OverlappedOp& = delete;

		// Movable
		OverlappedOp(OverlappedOp&& other) noexcept = default;
		auto operator=(OverlappedOp&& other) noexcept -> OverlappedOp& = default;

		auto WaitForCompletion(this auto&& self, Win32::DWORD timeout) -> bool
		{
			if (m_ioOverlapped == nullptr)
				throw Error::Boring32Error("IoOverlapped is null");
			bool successfulWait = self.m_ioEvent.WaitOnEvent(timeout, true);
			if (successfulWait)
				OnSuccess();
			return successfulWait;
		}

		auto GetWaitableHandle(this auto&& self) noexcept -> Win32::HANDLE
		{
			return self.m_ioEvent.GetHandle();
		}

		auto GetOverlapped(this auto&& self) -> Win32::OVERLAPPED*
		{
			if (self.m_ioOverlapped == nullptr)
				throw Error::Boring32Error("IoOverlapped is null");
			return self.m_ioOverlapped.get();
		}

		auto GetStatus(this auto&& self) -> std::uint64_t
		{
			if (self.m_ioOverlapped == nullptr)
				throw Error::Boring32Error("IoOverlapped is null");
			//STATUS_PENDING,
			//ERROR_IO_INCOMPLETE
			return self.m_lastError == Win32::ErrorCodes::IoPending ? self.m_ioOverlapped->Internal : self.m_lastError;
		}

		auto GetBytesTransferred(this auto&& self) noexcept -> std::uint64_t
		{
			if (self.m_ioOverlapped == nullptr)
				return 0;
			return self.m_ioOverlapped->InternalHigh;
		}

		auto IsReady(this auto&& self) -> bool
		{
			return self.m_ioOverlapped != nullptr;
		}

		auto IsComplete(this auto&& self) -> bool
		{
			if (self.m_ioOverlapped == nullptr)
				return false;
			return self.m_ioOverlapped->Internal != Win32::NTStatus::Pending;
		}

		auto IsSuccessful(this auto&& self) -> bool
		{
			if (self.m_ioOverlapped == nullptr)
				return false;
			// If the buffer is insufficient, Internal will be value 0x80000005L,
			// which is decimal value 2147483653. See error code STATUS_BUFFER_OVERFLOW:
			// https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
			return self.m_ioOverlapped->Internal == Win32::ErrorCodes::NoError;
		}

		auto IsPartial(this auto&& self) -> bool
		{
			if (self.m_ioOverlapped == nullptr)
				return false;
			return self.m_ioOverlapped->Internal == 0x80000005L;// STATUS_BUFFER_OVERFLOW;
		}

		void SetEvent(this auto&& self, bool signaled)
		{
			if (signaled)
				self.m_ioEvent.Signal();
			else
				self.m_ioEvent.Reset();
		}

		auto LastError(this auto&& self) -> Win32::DWORD
		{
			return self.m_lastError;
		}

		void LastError(this auto&& self, Win32::DWORD lastError)
		{
			self.m_lastError = lastError;
		}

	protected:
		void OnSuccess(this auto&& self) {}

		ManualResetEvent m_ioEvent{ false, false };
		std::unique_ptr<Win32::OVERLAPPED> m_ioOverlapped = std::make_unique<Win32::OVERLAPPED>();
		Win32::DWORD m_lastError = 0;
	};
}
