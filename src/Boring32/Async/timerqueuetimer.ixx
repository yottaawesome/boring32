export module boring32:async.timerqueuetimer;
import std;
import :win32;
import :error;
import :async.functions;

export namespace Boring32::Async
{
	// See https://learn.microsoft.com/en-us/windows/win32/sync/using-timer-queues
	class TimerQueueTimer
	{
	public:
		~TimerQueueTimer()
		{
			[[maybe_unused]] auto result = TryClose();
		}

		TimerQueueTimer() = default;
			
		TimerQueueTimer(
			Win32::HANDLE timerQueue,
			Win32::DWORD dueTime,
			Win32::DWORD period,
			Win32::DWORD flags,
			Win32::WAITORTIMERCALLBACK callback,
			void* parameter
		) : m_dueTime(dueTime),
			m_period(period),
			m_flags(flags),
			m_callback(callback),
			m_parameter(parameter),
			m_timerQueue(timerQueue),
			m_completionEvent(Win32::InvalidHandleValue)
		{
			if (not m_timerQueue or m_timerQueue == Win32::InvalidHandleValue)
				throw Error::Boring32Error{ "Invalid handle" };
			InternalCreate();
		}
			
		TimerQueueTimer(
			Win32::HANDLE timerQueue,
			Win32::DWORD dueTime,
			Win32::DWORD period,
			Win32::DWORD flags,
			Win32::WAITORTIMERCALLBACK callback,
			void* parameter,
			Win32::HANDLE completionEvent
		) : m_dueTime(dueTime),
			m_period(period),
			m_flags(flags),
			m_callback(callback),
			m_parameter(parameter),
			m_timerQueue(timerQueue),
			m_completionEvent(completionEvent),
			m_timerQueueTimer(nullptr)
		{
			if (not m_timerQueue or m_timerQueue == Win32::InvalidHandleValue)
				throw Error::Boring32Error{ "Invalid handle" };
			InternalCreate();
		}

		TimerQueueTimer(TimerQueueTimer&& other) noexcept
		{
			Move(other);
		}

		auto operator=(this auto&& self, TimerQueueTimer&& other) noexcept -> TimerQueueTimer&
		{
			Move(other);
			return *this;
		}

		TimerQueueTimer(const TimerQueueTimer& other) = delete;
		auto operator=(const TimerQueueTimer& other) -> TimerQueueTimer& = delete;

		void Update(this auto&& self, Win32::DWORD dueTime, Win32::DWORD period)
		{
			if (not m_timerQueueTimer or m_timerQueueTimer == Win32::InvalidHandleValue)
				throw Error::Boring32Error("m_timerQueueTimer is null");

			// https://learn.microsoft.com/en-us/windows/win32/api/threadpoollegacyapiset/nf-threadpoollegacyapiset-changetimerqueuetimer
			auto success = 
				Win32::ChangeTimerQueueTimer(
					m_timerQueue,
					m_timerQueueTimer,
					dueTime,
					period
				);
			if (not success)
				throw Error::Win32Error{Win32::GetLastError(), "ChangeTimerQueueTimer() failed"};
		}

		auto TryClose() -> std::expected<void, Win32::DWORD>
		{
			if (not m_timerQueueTimer or m_timerQueueTimer == Win32::InvalidHandleValue)
				return {};
			auto succeeded =
				Win32::DeleteTimerQueueTimer(
					m_timerQueue,
					m_timerQueueTimer,
					m_completionEvent
				);
			if (not succeeded)
				return std::unexpected{ Win32::GetLastError() };
			m_timerQueueTimer = nullptr;
			return {};
		}

		void Close(this auto&& self)
		{
			auto result = self.TryClose().or_else(
				[](Win32::DWORD errorCode) -> std::expected<void, Win32::DWORD>
				{
					throw Error::Win32Error{ errorCode, "DeleteTimerQueueTimer() failed" };
				});
		}

	protected:
		void Move(TimerQueueTimer& other) noexcept
		{
			m_dueTime = other.m_dueTime;
			m_period = other.m_period;
			m_flags = other.m_flags;
			m_callback = other.m_callback;
			m_parameter = other.m_parameter;

			m_timerQueue = std::exchange(other.m_timerQueue, nullptr);
			m_timerQueueTimer = std::exchange(other.m_timerQueueTimer, nullptr);
			m_completionEvent = std::exchange(other.m_completionEvent, nullptr);
		}

		void InternalCreate(this auto&& self)
		{
			if (not self.m_timerQueue or self.m_timerQueue == Win32::InvalidHandleValue)
				throw Error::Boring32Error{ "m_timerQueue is null" };

			//https://docs.microsoft.com/en-us/windows/win32/api/threadpoollegacyapiset/nf-threadpoollegacyapiset-createtimerqueuetimer
			auto succeeded = 
				Win32::CreateTimerQueueTimer(
					&self.m_timerQueueTimer,
					self.m_timerQueue,
					self.m_callback,
					self.m_parameter,
					self.m_dueTime,
					self.m_period,
					self.m_flags
				);
			if (not succeeded)
				throw Error::Win32Error{Win32::GetLastError(), "CreateTimerQueueTimer() failed"};
		}

		Win32::HANDLE m_timerQueue = nullptr;
		Win32::HANDLE m_timerQueueTimer = nullptr;
		Win32::HANDLE m_completionEvent = Win32::InvalidHandleValue;
		Win32::DWORD m_dueTime = 0;
		Win32::DWORD m_period = 0;
		Win32::DWORD m_flags = 0;
		Win32::WAITORTIMERCALLBACK m_callback = nullptr;
		void* m_parameter = nullptr;
	};
}

