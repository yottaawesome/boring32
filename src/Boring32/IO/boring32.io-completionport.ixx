module;

#include <Windows.h>

export module boring32.io:completionport;
import boring32.raii;

export namespace Boring32::IO
{
	// https://docs.microsoft.com/en-us/windows/win32/fileio/i-o-completion-ports
	class CompletionPort
	{
		public:
			virtual ~CompletionPort();
			CompletionPort(const unsigned maxThreads);

		public:
			virtual void Associate(HANDLE device, const ULONG_PTR completionKey);
			virtual HANDLE GetHandle() const noexcept;

		protected:
			RAII::Win32Handle m_completionPort;
	};
}