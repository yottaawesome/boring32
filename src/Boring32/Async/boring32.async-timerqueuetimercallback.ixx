export module boring32.async:timerqueuetimercallback;
import <iostream>;
import <win32.hpp>;
import :timerqueuetimer;
import boring32.error;

export namespace Boring32::Async
{
	class TimerQueueTimerCallback : public TimerQueueTimer
	{
		public:
			virtual ~TimerQueueTimerCallback() = default;

			TimerQueueTimerCallback() = default;
			
			TimerQueueTimerCallback(
				HANDLE timerQueue,
				const DWORD dueTime,
				const DWORD period,
				const DWORD flags
			) : TimerQueueTimer(timerQueue, dueTime, period, flags, InternalCallback, this)
			{ }
			
			TimerQueueTimerCallback(
				HANDLE timerQueue,
				const DWORD dueTime,
				const DWORD period,
				const DWORD flags,
				HANDLE completionEvent
			) : TimerQueueTimer(timerQueue, dueTime, period, flags, InternalCallback, this)
			{ }

			TimerQueueTimerCallback(TimerQueueTimerCallback&& other) noexcept
				: TimerQueueTimer(std::move(other))
			{ }

			virtual TimerQueueTimerCallback& operator=(
				TimerQueueTimerCallback&& other
			) noexcept 
			{
				Move(other);
				return *this;
			}

			TimerQueueTimerCallback(
				const TimerQueueTimerCallback& other
			) = delete;

			virtual TimerQueueTimerCallback& operator=(
				const TimerQueueTimerCallback& other
			) = delete;

		protected:
			virtual void Run(const BOOLEAN timerOrWaitFired) = 0;

		protected:
			static void InternalCallback(void* parameter, BOOLEAN timerOrWaitFired)
			{
				if (!parameter)
					throw Error::Boring32Error("Invalid parameter");
				TimerQueueTimerCallback* obj = (TimerQueueTimerCallback*)parameter;
				obj->Run(timerOrWaitFired);
			}
	};
}