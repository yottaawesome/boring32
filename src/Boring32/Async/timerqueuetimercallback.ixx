export module boring32:async.timerqueuetimercallback;
import std;
import :win32;
import :error;
import :async.timerqueuetimer;

export namespace Boring32::Async
{
	template<typename T>
	struct TimerQueueTimerCallback : public TimerQueueTimer
	{
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

		virtual auto operator=(TimerQueueTimerCallback&& other) noexcept -> TimerQueueTimerCallback&
		{
			Move(other);
			return *this;
		}

		TimerQueueTimerCallback(const TimerQueueTimerCallback&) = delete;
		virtual auto operator=(const TimerQueueTimerCallback&) -> TimerQueueTimerCallback& = delete;

		protected:
		static auto InternalCallback(void* parameter, BOOLEAN timerOrWaitFired) -> void
		{
			if (not parameter)
				throw Error::Boring32Error("Invalid parameter");
			auto* obj = reinterpret_cast<TimerQueueTimerCallback*>(parameter);
			T::Run(timerOrWaitFired);
		}
	};
}