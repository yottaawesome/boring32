#pragma once
#include "TimerQueueTimer.hpp"

namespace Boring32::Async
{
	class TimerQueueTimerCallback : public TimerQueueTimer
	{
		public:
			virtual ~TimerQueueTimerCallback();

			TimerQueueTimerCallback();
			
			TimerQueueTimerCallback(
				HANDLE timerQueue,
				const DWORD dueTime,
				const DWORD period,
				const DWORD flags
			);
			
			TimerQueueTimerCallback(
				HANDLE timerQueue,
				const DWORD dueTime,
				const DWORD period,
				const DWORD flags,
				HANDLE completionEvent
			);

			TimerQueueTimerCallback(TimerQueueTimerCallback&& other) noexcept;
			virtual TimerQueueTimerCallback& operator=(TimerQueueTimerCallback&& other) noexcept;

			TimerQueueTimerCallback(const TimerQueueTimerCallback& other) = delete;
			virtual TimerQueueTimerCallback& operator=(const TimerQueueTimerCallback& other) = delete;

		protected:
			virtual void Run(const BOOLEAN timerOrWaitFired) = 0;

		protected:
			static void InternalCallback(void* parameter, BOOLEAN timerOrWaitFired);
	};
}