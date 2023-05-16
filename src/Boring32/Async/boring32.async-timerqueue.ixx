module;

#include <source_location>;

export module boring32.async:timerqueue;
import :event;
import <stdexcept>;
import <win32.hpp>;
import boring32.error;
import <iostream>;

export namespace Boring32::Async
{
	//https://docs.microsoft.com/en-us/windows/win32/sync/timer-queues
	class TimerQueue
	{
		public:
			virtual ~TimerQueue()
			{
				Close(std::nothrow);
			}

			TimerQueue() = default;
			
			TimerQueue(TimerQueue&& other) noexcept
			{
				Move(other);
			}

			TimerQueue(const TimerQueue& other) = delete;

			TimerQueue(const Async::Event& completionEvent)
				: m_completionEvent(completionEvent),
				m_waitForAllCallbacks(true)
			{
				if (!m_completionEvent)
					throw Error::Boring32Error("completionEvent cannot be nullptr");
				if (m_completionEvent == INVALID_HANDLE_VALUE)
					throw Error::Boring32Error("completionEvent cannot be INVALID_HANDLE_VALUE");
				InternalCreate();
			}

			TimerQueue(const bool waitForAllCallbacks)
				: m_timer(nullptr),
				m_waitForAllCallbacks(waitForAllCallbacks)
			{
				InternalCreate();
			}
			
		public:
			virtual TimerQueue& operator=(TimerQueue&& other) noexcept
			{
				Move(other);
				return *this;
			}

			virtual TimerQueue& operator=(const TimerQueue& other) = delete;

		public:
			virtual void Close()
			{
				if (!m_timer)
					return;

				HANDLE argValue = INVALID_HANDLE_VALUE; // waits for all callbacks on deletion
				if (m_completionEvent)
					argValue = m_completionEvent.GetHandle(); // waits for all callbacks on deletion and signals event
				else if (!m_waitForAllCallbacks)
					argValue = nullptr; // does not wait

				//https://docs.microsoft.com/en-us/windows/win32/api/threadpoollegacyapiset/nf-threadpoollegacyapiset-deletetimerqueueex
				if (!DeleteTimerQueueEx(m_timer, argValue))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("DeleteTimerQueueEx() failed", lastError);
				}
				m_timer = nullptr;
			}

			virtual bool Close(const std::nothrow_t&) noexcept try
			{
				Close();
				return true;
			}
			catch (const std::exception& ex)
			{
				std::wcerr << ex.what() << std::endl;
				return false;
			}

			virtual HANDLE GetHandle() const noexcept final
			{
				return m_timer;
			}

		protected:
			virtual void InternalCreate()
			{
				//https://docs.microsoft.com/en-us/windows/win32/api/threadpoollegacyapiset/nf-threadpoollegacyapiset-createtimerqueue
				m_timer = CreateTimerQueue();
				if (m_timer == nullptr)
					throw Error::Win32Error("CreateTimerQueue() failed", GetLastError());
			}

			virtual void Move(TimerQueue& other) noexcept try
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

		protected:
			HANDLE m_timer = nullptr;
			Async::Event m_completionEvent;
			bool m_waitForAllCallbacks = false;
	};
}