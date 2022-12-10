module;

#include <stdexcept>

export module boring32.async:timerqueue;
import :event;
import <win32.hpp>;

export namespace Boring32::Async
{
	//https://docs.microsoft.com/en-us/windows/win32/sync/timer-queues
	class TimerQueue
	{
		public:
			virtual ~TimerQueue();
			TimerQueue();
			TimerQueue(TimerQueue&& other) noexcept;
			TimerQueue(const TimerQueue& other) = delete;

			TimerQueue(const Async::Event& completionEvent);
			TimerQueue(const bool waitForAllCallbacks);
			
		public:
			virtual TimerQueue& operator=(TimerQueue&& other) noexcept;
			virtual TimerQueue& operator=(const TimerQueue& other) = delete;

		public:
			virtual void Close();
			virtual bool Close(const std::nothrow_t) noexcept;
			virtual HANDLE GetHandle() const noexcept final;

		protected:
			virtual void InternalCreate();
			virtual void Move(TimerQueue& other) noexcept;

		protected:
			HANDLE m_timer;
			Async::Event m_completionEvent;
			bool m_waitForAllCallbacks;
	};
}