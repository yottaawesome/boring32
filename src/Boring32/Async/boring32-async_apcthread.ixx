export module boring32:async_apcthread;
import boring32.shared;
import :error;
import :async_thread;
import :async_event;

export namespace Boring32::Async
{
	// See https://learn.microsoft.com/en-us/windows/win32/sync/asynchronous-procedure-calls
	class APCThread : public Thread
	{
		public:
			using ApcFunctionSignature = Win32::PAPCFUNC;

		public:
			APCThread() = default;

		public:
			template<typename T, typename M>
			auto QueueInstanceAPC(const T& instance, M member) -> void
			{
				auto* arg = new InstanceMethod{
					.Instance = const_cast<T*>(&instance),
					.Method = member
				};

				QueueAPC(
					InternalAPC<T, M>,
					reinterpret_cast<Win32::ULONG_PTR>(arg)
				);
			}

			template<typename T>
			auto QueueAPC(const T& apc) -> void
				requires std::is_invocable_v<T>
			{
				QueueAPC(
					InternalAPC<T>,
					reinterpret_cast<Win32::ULONG_PTR>(const_cast<T*>(&apc))
				);
			}

			template<typename T>
			auto QueueAPC(T&& apc) -> void
				requires std::is_invocable_v<T>
			{
				QueueAPC(
					InternalHeapAPC<T>,
					reinterpret_cast<Win32::ULONG_PTR>(new T(std::move(apc)))
				);
			}

			auto QueueAPC(
				ApcFunctionSignature apc, 
				const Win32::ULONG_PTR arg
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

				const Win32::DWORD status = Win32::QueueUserAPC(
					apc,
					m_threadHandle,
					arg
				);
				if (!status)
				{
					const auto lastError = Win32::GetLastError();
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
					const Win32::DWORD waitResult = Win32::WaitForSingleObjectEx(
						m_wait.GetHandle(),
						Win32::Infinite,
						true
					);

					// This is our signal to exit
					if (waitResult == Win32::WaitObject0)
						return 0;
					// Results other than WAIT_IO_COMPLETION are unexpected
					if (waitResult != Win32::WaitIoCompletion)
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
			static void InternalAPC(Win32::ULONG_PTR arg)
				requires std::is_invocable_v<T>
			{
				const T& apc = *reinterpret_cast<T*>(arg);
				apc();
			}

			template<typename T>
			static void InternalHeapAPC(Win32::ULONG_PTR arg)
				requires std::is_invocable_v<T>
			{
				T* apc = reinterpret_cast<T*>(arg);
				(*apc)();
				delete apc;
			}

			template<typename T, typename M>
			struct InstanceMethod
			{
				T* Instance;
				M Method;
			};

			template<typename T, typename M>
			static void InternalAPC(Win32::ULONG_PTR arg)
			{
				auto* apc = reinterpret_cast<InstanceMethod<T, M>*>(arg);
				//(apc->Instance.*apc->Method)();
				std::invoke(apc->Method, apc->Instance);
				delete apc;
			}

		protected:
			ManualResetEvent m_wait{ false, false, };
	};
}
