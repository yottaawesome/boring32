export module boring32.async:timerqueuetimer;
import std;

import <win32.hpp>;
import boring32.error;
import :functions;
import :concepts;

export namespace Boring32::Async
{
	// See https://learn.microsoft.com/en-us/windows/win32/sync/using-timer-queues
	class TimerQueueTimer
	{
		public:
			virtual ~TimerQueueTimer()
			{
				Close(std::nothrow);
			}

			TimerQueueTimer() = default;
			
			TimerQueueTimer(
				HANDLE timerQueue,
				const DWORD dueTime,
				const DWORD period,
				const DWORD flags,
				WAITORTIMERCALLBACK callback,
				void* parameter
			) : m_dueTime(dueTime),
				m_period(period),
				m_flags(flags),
				m_callback(callback),
				m_parameter(parameter),
				m_timerQueue(timerQueue),
				m_completionEvent(INVALID_HANDLE_VALUE),
				m_timerQueueTimer(nullptr)
			{
				if (m_timerQueue == nullptr || m_timerQueue == INVALID_HANDLE_VALUE)
					throw Error::Boring32Error("Invalid handle");
				InternalCreate();
			}
			
			TimerQueueTimer(
				HANDLE timerQueue,
				const DWORD dueTime,
				const DWORD period,
				const DWORD flags,
				WAITORTIMERCALLBACK callback,
				void* parameter,
				HANDLE completionEvent
			) : m_dueTime(dueTime),
				m_period(period),
				m_flags(flags),
				m_callback(callback),
				m_parameter(parameter),
				m_timerQueue(timerQueue),
				m_completionEvent(completionEvent),
				m_timerQueueTimer(nullptr)
			{
				if (m_timerQueue == nullptr || m_timerQueue == INVALID_HANDLE_VALUE)
					throw Error::Boring32Error("Invalid handle");
				InternalCreate();
			}

			TimerQueueTimer(TimerQueueTimer&& other) noexcept
				: m_timerQueueTimer(nullptr),
				m_timerQueue(nullptr),
				m_completionEvent(nullptr)
			{
				Move(other);
			}

			virtual TimerQueueTimer& operator=(TimerQueueTimer&& other) noexcept
			{
				Move(other);
				return *this;
			}

			TimerQueueTimer(const TimerQueueTimer& other) = delete;
			virtual TimerQueueTimer& operator=(const TimerQueueTimer& other) = delete;

		public:
			virtual void Update(const ULONG dueTime, const ULONG period)
			{
				if (!m_timerQueueTimer || m_timerQueueTimer == INVALID_HANDLE_VALUE)
					throw Error::Boring32Error("m_timerQueueTimer is null");

				// https://learn.microsoft.com/en-us/windows/win32/api/threadpoollegacyapiset/nf-threadpoollegacyapiset-changetimerqueuetimer
				const bool success = ChangeTimerQueueTimer(
					m_timerQueue,
					m_timerQueueTimer,
					dueTime,
					period
				);
				if (!success)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("ChangeTimerQueueTimer() failed", lastError);
				}
			}

			virtual void Close()
			{
				if (!m_timerQueueTimer || m_timerQueueTimer != INVALID_HANDLE_VALUE)
					return;
				// https://learn.microsoft.com/en-us/windows/win32/api/threadpoollegacyapiset/nf-threadpoollegacyapiset-deletetimerqueuetimer
				const bool succeeded = DeleteTimerQueueTimer(
					m_timerQueue,
					m_timerQueueTimer,
					m_completionEvent
				);
				if (!succeeded)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("DeleteTimerQueueTimer() failed", lastError);
				}
				m_timerQueueTimer = nullptr;
			}

			virtual bool Close(
				const std::nothrow_t&
			) noexcept try
			{
				Close();
				return true;
			}
			catch (const std::exception& ex)
			{
				// ICE
				std::wcerr << ex.what() << std::endl;
				return false;
			}

		protected:
			virtual void Move(TimerQueueTimer& other) noexcept
			{
				m_dueTime = other.m_dueTime;
				m_period = other.m_period;
				m_flags = other.m_flags;
				m_callback = other.m_callback;
				m_parameter = other.m_parameter;

				m_timerQueue = other.m_timerQueue;
				m_timerQueueTimer = other.m_timerQueueTimer;
				m_completionEvent = other.m_completionEvent;
				other.m_timerQueue = nullptr;
				other.m_timerQueueTimer = nullptr;
				other.m_completionEvent = nullptr;
			}

			virtual void InternalCreate()
			{
				if (!m_timerQueue || m_timerQueue == INVALID_HANDLE_VALUE)
					throw Error::Boring32Error("m_timerQueue is null");

				//https://docs.microsoft.com/en-us/windows/win32/api/threadpoollegacyapiset/nf-threadpoollegacyapiset-createtimerqueuetimer
				const bool succeeded = CreateTimerQueueTimer(
					&m_timerQueueTimer,
					m_timerQueue,
					m_callback,
					m_parameter,
					m_dueTime,
					m_period,
					m_flags
				);
				if (!succeeded)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("CreateTimerQueueTimer() failed", lastError);
				}
			}

		protected:
			HANDLE m_timerQueue = nullptr;
			HANDLE m_timerQueueTimer = nullptr;
			HANDLE m_completionEvent = INVALID_HANDLE_VALUE;
			DWORD m_dueTime = 0;
			DWORD m_period = 0;
			DWORD m_flags = 0;
			WAITORTIMERCALLBACK m_callback = nullptr;
			void* m_parameter = nullptr;
	};
}

