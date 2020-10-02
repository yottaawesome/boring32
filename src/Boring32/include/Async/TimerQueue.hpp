#pragma once
#include <Windows.h>
#include "TimerQueueTimer.hpp"

namespace Boring32::Async
{
	//https://docs.microsoft.com/en-us/windows/win32/sync/timer-queues
	class TimerQueue
	{
		public:
			virtual ~TimerQueue();
			TimerQueue();
			TimerQueue(const HANDLE completionEvent);

			TimerQueue(TimerQueue&& other) noexcept;
			virtual TimerQueue& operator=(TimerQueue&& other) noexcept;

			TimerQueue(const TimerQueue& other) = delete;
			virtual TimerQueue& operator=(const TimerQueue& other) = delete;

		public:
			virtual void Close();
			virtual bool Close(const std::nothrow_t) noexcept;
			virtual HANDLE GetHandle() const;

		protected:
			virtual void InternalCreate();
			virtual void Move(TimerQueue& other) noexcept;

		protected:
			HANDLE m_timer;
			HANDLE m_completionEvent;
	};
}