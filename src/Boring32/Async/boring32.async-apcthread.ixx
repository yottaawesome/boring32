export module boring32.async:apcthread;
import <iostream>;
import <format>;
import <win32.hpp>;
import boring32.error;
import :thread;
import :event;

export namespace Boring32::Async
{
	class APCThread : public Thread
	{
		public:
			using ApcFunctionSignature = PAPCFUNC;

		public:
			APCThread() = default;

		public:
			auto QueueAPC(ApcFunctionSignature apc, const ULONG_PTR arg) -> void
			{
				if (m_status != ThreadStatus::Running && m_status != ThreadStatus::Suspended)
					throw Error::Boring32Error(
						std::format(
							"APCThread must either be running or suspended. Currently in {}.",
							int(m_status)
						)
					);

				const DWORD status = QueueUserAPC(
					apc,
					GetHandle(),
					arg
				);
				if (!status)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("QueueUserAPC() failed", lastError);
				}
			}

			auto SignalToExit() -> void
			{
				m_wait.Signal();
			}

		protected:
			auto Run() -> unsigned override
			{
				while (true)
				{
					// Do an alertable wait
					const DWORD waitResult = WaitForSingleObjectEx(
						m_wait.GetHandle(),
						INFINITE,
						true
					);

					// This is our signal to exit
					if (waitResult == WAIT_OBJECT_0)
						return 0;
					// Results other than WAIT_IO_COMPLETION are unexpected
					if (waitResult != WAIT_IO_COMPLETION)
					{
						std::wcerr << std::format(
							L"Unexpected WaitForSingleObjectEx(): {}",
							waitResult
						);
						return waitResult;
					}
				}
				
				return 0;
			}

		protected:
			Event m_wait{ false, true, false, };
	};
}
