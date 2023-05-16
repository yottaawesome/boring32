module;

#include <source_location>;

export module boring32.io:completionport;
import <win32.hpp>;
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
				m_completionPort = CreateIoCompletionPort(
					INVALID_HANDLE_VALUE,
					nullptr,
					0,
					maxThreads
				);
			}

		public:
			virtual void Associate(HANDLE device, const ULONG_PTR completionKey)
			{
				if (!device)
					throw Error::Boring32Error("device cannot be null");

				if (!CreateIoCompletionPort(device, m_completionPort.GetHandle(), completionKey, 0))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("CreateIoCompletionPort() failed", lastError);
				}
			}

			virtual HANDLE GetHandle() const noexcept
			{
				return m_completionPort;
			}

			virtual bool GetCompletionStatus()
			{
				DWORD bytesTransferred = 0;
				ULONG_PTR completionKey = 0;
				OVERLAPPED* overlapped = nullptr;
				// https://learn.microsoft.com/en-us/windows/win32/api/ioapiset/nf-ioapiset-getqueuedcompletionstatus
				// See also https://learn.microsoft.com/en-us/windows/win32/fileio/getqueuedcompletionstatusex-func
				const bool success = GetQueuedCompletionStatus(
					m_completionPort,
					&bytesTransferred,
					&completionKey,
					&overlapped,
					INFINITE
				);

				if (!success)
				{
					const DWORD lastError = GetLastError();
					if (!overlapped)
					{
						if (lastError != ERROR_ABANDONED_WAIT_0)
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