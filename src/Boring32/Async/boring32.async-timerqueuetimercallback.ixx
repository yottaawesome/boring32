export module boring32.async:timerqueuetimercallback;
import std;
import std.compat;
import boring32.win32;
import boring32.error;
import :timerqueuetimer;

export namespace Boring32::Async
{
	template<typename T>
	class TimerQueueTimerCallback : public TimerQueueTimer
	{
		public:
			virtual ~TimerQueueTimerCallback() = default;

			TimerQueueTimerCallback() = default;
			
			TimerQueueTimerCallback(
				Win32::HANDLE timerQueue,
				const Win32::DWORD dueTime,
				const Win32::DWORD period,
				const Win32::DWORD flags
			) : TimerQueueTimer(timerQueue, dueTime, period, flags, InternalCallback, this)
			{ }
			
			TimerQueueTimerCallback(
				Win32::HANDLE timerQueue,
				const Win32::DWORD dueTime,
				const Win32::DWORD period,
				const Win32::DWORD flags,
				Win32::HANDLE completionEvent
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
			static void InternalCallback(void* parameter, BOOLEAN timerOrWaitFired)
			{
				if (!parameter)
					throw Error::Boring32Error("Invalid parameter");
				auto* obj = reinterpret_cast<TimerQueueTimerCallback*>(parameter);
				T::Run(timerOrWaitFired);
			}
	};
}