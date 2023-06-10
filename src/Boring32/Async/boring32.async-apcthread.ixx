export module boring32.async:apcthread;
import <iostream>;
import <format>;
import <functional>;
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
			template<typename T>
			auto QueueAPC(const T& apc) -> void
			{
				QueueAPC(
					InternalAPC<T>,
					reinterpret_cast<ULONG_PTR>(const_cast<T*>(&apc))
				);
			}

			auto QueueAPC(
				ApcFunctionSignature apc, 
				const ULONG_PTR arg
			) -> void
			{
				if (!m_threadHandle)
					throw Error::Boring32Error("No thread handle found. Either the thread hasn't been started or has been Close()d.");
				if (m_status != ThreadStatus::Running && m_status != ThreadStatus::Suspended)
					throw Error::Boring32Error(
						std::format(
							"APCThread must either be running or suspended. Currently in {}.",
							int(m_status)
						)
					);

				const DWORD status = QueueUserAPC(
					apc,
					m_threadHandle,
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

			template<typename T>
			static void InternalAPC(ULONG_PTR arg)
			{
				const T& apc = *reinterpret_cast<T*>(arg);
				apc();
			}

		protected:
			Event m_wait{ false, true, false, };
	};
}
