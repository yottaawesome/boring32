export module boring32:async_timerqueue;
import boring32.shared;
import :error;
import :async_event;

export namespace Boring32::Async
{
	//https://docs.microsoft.com/en-us/windows/win32/sync/timer-queues
	template<Async::AnyEvent TEvent = Async::AutoResetEvent>
	struct TimerQueue final
	{
		~TimerQueue()
		{
			Close(std::nothrow);
		}

		TimerQueue() = default;
			
		TimerQueue(TimerQueue&& other) noexcept
		{
			Move(other);
		}

		TimerQueue(const TimerQueue& other) = delete;

		TimerQueue(const TEvent& completionEvent)
			: m_completionEvent(completionEvent),
			m_waitForAllCallbacks(true)
		{
			if (!m_completionEvent)
				throw Error::Boring32Error("completionEvent cannot be nullptr");
			if (m_completionEvent == Win32::InvalidHandleValue)
				throw Error::Boring32Error("completionEvent cannot be INVALID_HANDLE_VALUE");
			InternalCreate();
		}

		TimerQueue(const bool waitForAllCallbacks)
			: m_timer(nullptr),
			m_waitForAllCallbacks(waitForAllCallbacks)
		{
			InternalCreate();
		}
			
		TimerQueue& operator=(TimerQueue&& other) noexcept
		{
			Move(other);
			return *this;
		}

		TimerQueue& operator=(const TimerQueue& other) = delete;

		void Close()
		{
			if (!m_timer)
				return;

			Win32::HANDLE argValue = Win32::InvalidHandleValue; // waits for all callbacks on deletion
			if (m_completionEvent.has_value() and m_completionEvent.value())
				argValue = m_completionEvent->GetHandle(); // waits for all callbacks on deletion and signals event
			else if (!m_waitForAllCallbacks)
				argValue = nullptr; // does not wait

			//https://docs.microsoft.com/en-us/windows/win32/api/threadpoollegacyapiset/nf-threadpoollegacyapiset-deletetimerqueueex
			if (!Win32::DeleteTimerQueueEx(m_timer, argValue))
				throw Error::Win32Error(Win32::GetLastError(), "DeleteTimerQueueEx() failed");
			m_timer = nullptr;
		}

		bool Close(const std::nothrow_t&) noexcept 
		try
		{
			Close();
			return true;
		}
		catch (const std::exception& ex)
		{
			std::wcerr << ex.what() << std::endl;
			return false;
		}

		Win32::HANDLE GetHandle() const noexcept
		{
			return m_timer;
		}

		private:
		void InternalCreate()
		{
			//https://docs.microsoft.com/en-us/windows/win32/api/threadpoollegacyapiset/nf-threadpoollegacyapiset-createtimerqueue
			m_timer = Win32::CreateTimerQueue();
			if (m_timer == nullptr)
			{
				const auto lastError = Win32::GetLastError();
				throw Error::Win32Error("CreateTimerQueue() failed", lastError);
			}
		}

		void Move(TimerQueue& other) noexcept 
		try
		{
			Close(std::nothrow);
			m_completionEvent = other.m_completionEvent;
			m_timer = other.m_timer;
			other.m_timer = nullptr;
		}
		catch (const std::exception& ex)
		{
			std::wcerr << ex.what() << std::endl;
		}

		Win32::HANDLE m_timer = nullptr;
		std::optional<TEvent> m_completionEvent;
		bool m_waitForAllCallbacks = false;
	};
}