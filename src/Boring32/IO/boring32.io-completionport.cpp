module;

#include <source_location>
#include <Windows.h>

module boring32.io:completionport;
import boring32.error;

namespace Boring32::IO
{
	CompletionPort::~CompletionPort() {}
	CompletionPort::CompletionPort(const unsigned maxThreads)
	{
		// https://docs.microsoft.com/en-us/windows/win32/fileio/createiocompletionport
		m_completionPort = CreateIoCompletionPort(
			INVALID_HANDLE_VALUE, 
			nullptr, 
			0, 
			maxThreads
		);
	}

	void CompletionPort::Associate(HANDLE device, const ULONG_PTR completionKey)
	{
		if (!CreateIoCompletionPort(device, m_completionPort.GetHandle(), completionKey, 0))
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("CreateIoCompletionPort() failed", lastError);
		}
	}
}