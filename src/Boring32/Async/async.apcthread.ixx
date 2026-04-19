export module boring32:async.apcthread;
import std;
import :win32;
import :error;
import :async.thread;
import :async.event;

export namespace Boring32::Async
{
	// See https://learn.microsoft.com/en-us/windows/win32/sync/asynchronous-procedure-calls
	class APCThread : public Thread
	{
	public:
		using ApcFunctionSignature = Win32::PAPCFUNC;
		APCThread() = default;

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

		template<std::invocable T>
		auto QueueAPC(const T& apc) -> void
		{
			QueueAPC(
				InternalAPC<T>,
				reinterpret_cast<Win32::ULONG_PTR>(const_cast<T*>(&apc))
			);
		}

		template<std::invocable T>
		auto QueueAPC(T&& apc) -> void
		{
			QueueAPC(
				InternalHeapAPC<T>,
				reinterpret_cast<Win32::ULONG_PTR>(new T(std::move(apc)))
			);
		}

		auto QueueAPC(ApcFunctionSignature apc, const Win32::ULONG_PTR arg) -> void
		{
			if (not m_threadHandle)
				throw Error::Boring32Error("No thread handle found. Either the thread hasn't been started or has been Close()d.");
			if (m_status != ThreadStatus::Running && m_status != ThreadStatus::Suspended)
				throw Error::Boring32Error(
					std::format(
						"APCThread must either be running or suspended. Currently in {}.",
						int(m_status)
					)
				);
			if (not Win32::QueueUserAPC(apc, m_threadHandle, arg))
				throw Error::Win32Error{ Win32::GetLastError(), "QueueUserAPC() failed" };
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
				Win32::DWORD waitResult = Win32::WaitForSingleObjectEx(
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
					std::wcerr << std::format(L"Unexpected WaitForSingleObjectEx(): {}", waitResult);
					return waitResult;
				}
			}
				
			return 0;
		}

		template<std::invocable T>
		static auto InternalAPC(Win32::ULONG_PTR arg) -> void
		{
			const T& apc = *reinterpret_cast<T*>(arg);
			apc();
		}

		template<std::invocable T>
		static auto InternalHeapAPC(Win32::ULONG_PTR arg) -> void
		{
			if (not arg)
				return;
			// ensure we delete the heap allocation after invoking the APC in an exception-safe way
			auto apc = std::unique_ptr<T>{ reinterpret_cast<T*>(arg) };
			(*apc)();
		}

		template<typename T, typename M>
		struct InstanceMethod
		{
			T* Instance;
			M Method;
		};

		template<typename T, typename M>
		static auto InternalAPC(Win32::ULONG_PTR arg) -> void
		{
			if (not arg)
				return;
			// ensure we delete the heap allocation after invoking the APC in an exception-safe way
			auto apc = std::unique_ptr<InstanceMethod<T, M>>{ reinterpret_cast<InstanceMethod<T, M>*>(arg) };
			//(apc->Instance.*apc->Method)();
			std::invoke(apc->Method, apc->Instance);
		}

		ManualResetEvent m_wait{ false, false, };
	};
}
