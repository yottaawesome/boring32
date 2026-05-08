export module boring32:io.completionport;
import :win32;
import :raii;
import :error;

export namespace Boring32::IO
{
	// https://docs.microsoft.com/en-us/windows/win32/fileio/i-o-completion-ports
	class CompletionPort final
	{
	public:
		CompletionPort(const unsigned maxThreads)
		{
			// https://docs.microsoft.com/en-us/windows/win32/fileio/createiocompletionport
			m_completionPort = 
				Win32::CreateIoCompletionPort(
					Win32::InvalidHandleValue,
					nullptr,
					0,
					maxThreads
				);
		}

		void Associate(Win32::HANDLE device, const Win32::ULONG_PTR completionKey)
		{
			if (not device)
				throw Error::Boring32Error{ "device cannot be null" };
			if (not Win32::CreateIoCompletionPort(device, m_completionPort.GetHandle(), completionKey, 0))
				throw Error::Win32Error{Win32::GetLastError(), "CreateIoCompletionPort() failed"};
		}

		auto GetHandle() const noexcept -> Win32::HANDLE
		{
			return m_completionPort;
		}

		auto GetCompletionStatus() -> bool
		{
			auto bytesTransferred = Win32::DWORD{};
			auto completionKey = Win32::ULONG_PTR{};
			auto overlapped = (Win32::OVERLAPPED*)nullptr;
			// https://learn.microsoft.com/en-us/windows/win32/api/ioapiset/nf-ioapiset-getqueuedcompletionstatus
			// See also https://learn.microsoft.com/en-us/windows/win32/fileio/getqueuedcompletionstatusex-func
			auto success = Win32::GetQueuedCompletionStatus(
				m_completionPort,
				&bytesTransferred,
				&completionKey,
				&overlapped,
				Win32::Infinite
			);
			if (not success)
			{
				if (auto lastError = Win32::GetLastError(); not overlapped and lastError != Win32::ErrorCodes::AbandonedWait0)
					throw Error::Win32Error{lastError, "GetQueuedCompletionStatus() failed"};
				else {}
				// Dequeued a completion packet for a failed I/O operation.
				// Not really clear what we should do here.
			}

			return success;
		}

	private:
		RAII::SharedHandle m_completionPort;
	};
}