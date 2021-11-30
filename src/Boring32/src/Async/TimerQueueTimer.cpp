module;

#include <iostream>
#include <stdexcept>
#include <Windows.h>

module boring32.async.timerqueuetimer;
import boring32.error.win32error;

namespace Boring32::Async
{
	TimerQueueTimer::~TimerQueueTimer()
	{
		Close(std::nothrow);
	}

	TimerQueueTimer::TimerQueueTimer()
	:	m_dueTime(0),
		m_period(0),
		m_flags(0),
		m_callback(nullptr),
		m_parameter(nullptr),
		m_timerQueue(nullptr),
		m_completionEvent(INVALID_HANDLE_VALUE),
		m_timerQueueTimer(nullptr)
	{ }

	TimerQueueTimer::TimerQueueTimer(
		HANDLE timerQueue,
		const DWORD dueTime,
		const DWORD period,
		const DWORD flags,
		WAITORTIMERCALLBACK callback,
		void* parameter
	)
	:	m_dueTime(dueTime),
		m_period(period),
		m_flags(flags),
		m_callback(callback),
		m_parameter(parameter),
		m_timerQueue(timerQueue),
		m_completionEvent(INVALID_HANDLE_VALUE),
		m_timerQueueTimer(nullptr)
	{
		if (m_timerQueue == nullptr || m_timerQueue == INVALID_HANDLE_VALUE)
			throw std::runtime_error("TimerQueueTimer::TimerQueueTimer(): Invalid handle");
		InternalCreate();
	}

	TimerQueueTimer::TimerQueueTimer(
		HANDLE timerQueue,
		const DWORD dueTime,
		const DWORD period,
		const DWORD flags,
		WAITORTIMERCALLBACK callback,
		void* parameter,
		HANDLE completionEvent
	)
	:	m_dueTime(dueTime),
		m_period(period),
		m_flags(flags),
		m_callback(callback),
		m_parameter(parameter),
		m_timerQueue(timerQueue),
		m_completionEvent(completionEvent),
		m_timerQueueTimer(nullptr)
	{
		if (m_timerQueue == nullptr || m_timerQueue == INVALID_HANDLE_VALUE)
			throw std::invalid_argument("TimerQueueTimer::TimerQueueTimer(): Invalid handle");
		InternalCreate();
	}

	TimerQueueTimer::TimerQueueTimer(TimerQueueTimer&& other) noexcept
	:	m_timerQueueTimer(nullptr),
		m_timerQueue(nullptr),
		m_completionEvent(nullptr)
	{
		Move(other);
	}
	
	TimerQueueTimer& TimerQueueTimer::operator=(TimerQueueTimer&& other) noexcept
	{
		Move(other);
		return *this;
	}

	void TimerQueueTimer::Update(const ULONG dueTime, const ULONG period)
	{
		if (m_timerQueueTimer == nullptr || m_timerQueueTimer == INVALID_HANDLE_VALUE)
			throw std::runtime_error(__FUNCSIG__": m_timerQueueTimer is null");

		bool success = ChangeTimerQueueTimer(
			m_timerQueue,
			m_timerQueueTimer,
			dueTime,
			period
		);
		if (success == false)
			throw Error::Win32Error(__FUNCSIG__": ChangeTimerQueueTimer() failed", GetLastError());
	}

	void TimerQueueTimer::Close()
	{
		if (m_timerQueueTimer && m_timerQueueTimer != INVALID_HANDLE_VALUE)
		{
			bool succeeded = DeleteTimerQueueTimer(
				m_timerQueue,
				m_timerQueueTimer,
				m_completionEvent
			);
			if (succeeded == false)
				throw Error::Win32Error(__FUNCSIG__": DeleteTimerQueueTimer() failed", GetLastError());
			m_timerQueueTimer = nullptr;
		}
	}

	bool TimerQueueTimer::Close(const std::nothrow_t) noexcept
	{
		try
		{
			Close();
			return true;
		}
		catch (const std::exception& ex)
		{
			std::wcerr << ex.what() << std::endl;
			return false;
		}
	}

	void TimerQueueTimer::Move(TimerQueueTimer& other) noexcept
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

	void TimerQueueTimer::InternalCreate()
	{
		if (m_timerQueue == nullptr || m_timerQueue == INVALID_HANDLE_VALUE)
			throw std::runtime_error(__FUNCSIG__": m_timerQueue is null");

		//https://docs.microsoft.com/en-us/windows/win32/api/threadpoollegacyapiset/nf-threadpoollegacyapiset-createtimerqueuetimer
		bool succeeded = CreateTimerQueueTimer(
			&m_timerQueueTimer,
			m_timerQueue,
			m_callback,
			m_parameter,
			m_dueTime,
			m_period,
			m_flags
		);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__": CreateTimerQueueTimer() failed", GetLastError());
	}
}
