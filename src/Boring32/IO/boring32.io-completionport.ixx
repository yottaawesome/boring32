export module boring32.io:completionport;
import boring32.win32;
import boring32.raii;
import boring32.error;

export namespace Boring32::IO
{
	// https://docs.microsoft.com/en-us/windows/win32/fileio/i-o-completion-ports
	class CompletionPort
	{
		public:
			virtual ~CompletionPort() = default;
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

		public:
			virtual void Associate(Win32::HANDLE device, const Win32::ULONG_PTR completionKey)
			{
				if (!device)
					throw Error::Boring32Error("device cannot be null");

				if (!Win32::CreateIoCompletionPort(device, m_completionPort.GetHandle(), completionKey, 0))
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("CreateIoCompletionPort() failed", lastError);
				}
			}

			virtual Win32::HANDLE GetHandle() const noexcept
			{
				return m_completionPort;
			}

			virtual bool GetCompletionStatus()
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

				if (!success)
				{
					const Win32::DWORD lastError = Win32::GetLastError();
					if (!overlapped)
					{
						if (lastError != Win32::ErrorCodes::AbandonedWait0)
							throw Error::Win32Error("GetQueuedCompletionStatus() failed", lastError);
					}
					else
					{
						// Dequeued a completion packet for a failed I/O operation.
						// Not really clear what we should do here.
					}
				}

				return success;
			}

		protected:
			RAII::Win32Handle m_completionPort;
	};
}