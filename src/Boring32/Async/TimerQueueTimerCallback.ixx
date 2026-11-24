export module boring32:async.timerqueuetimercallback;
import std;
import :win32;
import :error;
import :async.timerqueuetimer;

export namespace Boring32::Async
{
	template<typename T>
	class TimerQueueTimerCallback : public TimerQueueTimer
	{
	public:
		TimerQueueTimerCallback() = default;

		auto operator=(TimerQueueTimerCallback&& other) noexcept -> TimerQueueTimerCallback&
		{
			Move(other);
			return *this;
		}

		TimerQueueTimerCallback(const TimerQueueTimerCallback&) = delete;
		auto operator=(const TimerQueueTimerCallback&) -> TimerQueueTimerCallback & = delete;
			
		TimerQueueTimerCallback(
			Win32::HANDLE timerQueue,
			Win32::DWORD dueTime,
			Win32::DWORD period,
			Win32::DWORD flags
		) : TimerQueueTimer(timerQueue, dueTime, period, flags, InternalCallback, this)
		{ }
			
		TimerQueueTimerCallback(
			Win32::HANDLE timerQueue,
			Win32::DWORD dueTime,
			Win32::DWORD period,
			Win32::DWORD flags,
			Win32::HANDLE completionEvent
		) : TimerQueueTimer(timerQueue, dueTime, period, flags, InternalCallback, this, completionEvent)
		{ }

		TimerQueueTimerCallback(TimerQueueTimerCallback&& other) noexcept
			: TimerQueueTimer(std::move(other))
		{ }

	protected:
		static void InternalCallback(void* parameter, BOOLEAN timerOrWaitFired)
		{
			if (not parameter)
				throw Error::Boring32Error{ "Invalid parameter" };
			auto* obj = reinterpret_cast<TimerQueueTimerCallback*>(parameter);
			T::Run(timerOrWaitFired);
		}
	};
}