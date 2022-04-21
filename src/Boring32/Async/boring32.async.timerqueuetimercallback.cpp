module;

#include <iostream>
#include <Windows.h>

module boring32.async.timerqueuetimercallback;
import boring32.error;

namespace Boring32::Async
{
	void TimerQueueTimerCallback::InternalCallback(void* parameter, BOOLEAN timerOrWaitFired)
	{
		if (parameter == nullptr)
			throw std::invalid_argument("Invalid parameter");
		TimerQueueTimerCallback* obj = (TimerQueueTimerCallback*)parameter;
		obj->Run(timerOrWaitFired);
	}

	TimerQueueTimerCallback::~TimerQueueTimerCallback()
	{ }

	TimerQueueTimerCallback::TimerQueueTimerCallback()
	{ }

	TimerQueueTimerCallback::TimerQueueTimerCallback(
		HANDLE timerQueue,
		const DWORD dueTime,
		const DWORD period,
		const DWORD flags
	)
	:	TimerQueueTimer(timerQueue, dueTime, period, flags, InternalCallback, this)
	{ }

	TimerQueueTimerCallback::TimerQueueTimerCallback(
		HANDLE timerQueue,
		const DWORD dueTime,
		const DWORD period,
		const DWORD flags,
		HANDLE completionEvent
	)
	:	TimerQueueTimer(timerQueue, dueTime, period, flags, InternalCallback, this)
	{ }

	TimerQueueTimerCallback::TimerQueueTimerCallback(TimerQueueTimerCallback&& other) noexcept
	:	TimerQueueTimer(std::move(other))
	{ }

	TimerQueueTimerCallback& TimerQueueTimerCallback::operator=(TimerQueueTimerCallback&& other) noexcept
	{
		Move(other);
		return *this;
	}
}