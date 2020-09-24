#pragma once
#include <Windows.h>

namespace Boring32::Async
{
	class TimerQueueTimer
	{
		public:
			virtual ~TimerQueueTimer();

			TimerQueueTimer();
			
			TimerQueueTimer(
				HANDLE timerQueue,
				const DWORD dueTime,
				const DWORD period,
				const DWORD flags,
				WAITORTIMERCALLBACK callback,
				void* parameter
			);
			
			TimerQueueTimer(
				HANDLE timerQueue,
				const DWORD dueTime,
				const DWORD period,
				const DWORD flags,
				WAITORTIMERCALLBACK callback,
				void* parameter,
				HANDLE completionEvent
			);

			TimerQueueTimer(TimerQueueTimer&& other) noexcept;
			virtual TimerQueueTimer& operator=(TimerQueueTimer&& other) noexcept;

			TimerQueueTimer(const TimerQueueTimer& other) = delete;
			virtual TimerQueueTimer& operator=(const TimerQueueTimer& other) = delete;

		public:
			virtual void Update(const ULONG dueTime, const ULONG period);
			virtual void Close();
			virtual bool Close(const std::nothrow_t) noexcept;

		protected:
			virtual void Move(TimerQueueTimer& other) noexcept;
			virtual void InternalCreate();

		protected:
			HANDLE m_timerQueue;
			HANDLE m_timerQueueTimer;
			HANDLE m_completionEvent;
			DWORD m_dueTime;
			DWORD m_period;
			DWORD m_flags;
			WAITORTIMERCALLBACK m_callback;
			void* m_parameter;
	};
}