export module boring32.io:completionport;
import boring32.raii;
import <Windows.h>;

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
			virtual bool GetCompletionStatus();

		protected:
			RAII::Win32Handle m_completionPort;
	};
}