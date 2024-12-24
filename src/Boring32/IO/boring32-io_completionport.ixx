export module boring32:io_completionport;
import boring32.shared;
import :raii;
import :error;

export namespace Boring32::IO
{
	// https://docs.microsoft.com/en-us/windows/win32/fileio/i-o-completion-ports
	struct CompletionPort final
	{
		CompletionPort(const unsigned maxThreads)
		{
			// https://docs.microsoft.com/en-us/windows/win32/fileio/createiocompletionport
			m_completionPort = Win32::CreateIoCompletionPort(
				Win32::InvalidHandleValue,
				nullptr,
				0,
				maxThreads
			);
		}

		void Associate(Win32::HANDLE device, const Win32::ULONG_PTR completionKey)
		{
			if (not device)
				throw Error::Boring32Error("device cannot be null");
			if (auto lastError = Win32::GetLastError(); not Win32::CreateIoCompletionPort(device, m_completionPort.GetHandle(), completionKey, 0))
				throw Error::Win32Error("CreateIoCompletionPort() failed", lastError);
		}

		Win32::HANDLE GetHandle() const noexcept
		{
			return m_completionPort;
		}

		bool GetCompletionStatus()
		{
			Win32::DWORD bytesTransferred = 0;
			Win32::ULONG_PTR completionKey = 0;
			Win32::OVERLAPPED* overlapped = nullptr;
			// https://learn.microsoft.com/en-us/windows/win32/api/ioapiset/nf-ioapiset-getqueuedcompletionstatus
			// See also https://learn.microsoft.com/en-us/windows/win32/fileio/getqueuedcompletionstatusex-func
			const bool success = Win32::GetQueuedCompletionStatus(
				m_completionPort,
				&bytesTransferred,
				&completionKey,
				&overlapped,
				Win32::Infinite
			);

			if (Win32::DWORD lastError = Win32::GetLastError(); not success)
			{
				if (not overlapped and lastError != Win32::ErrorCodes::AbandonedWait0)
					throw Error::Win32Error("GetQueuedCompletionStatus() failed", lastError);
				else {}
				// Dequeued a completion packet for a failed I/O operation.
				// Not really clear what we should do here.
			}

			return success;
		}

		private:
		RAII::Win32Handle m_completionPort;
	};
}