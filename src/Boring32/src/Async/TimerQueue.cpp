module;

#include <iostream>
#include <stdexcept>
#include <Windows.h>

module boring32.async.timerqueue;
import boring32.error.win32error;

namespace Boring32::Async
{
	TimerQueue::~TimerQueue()
	{
		Close(std::nothrow);
	}

	TimerQueue::TimerQueue()
	:	m_timer(nullptr),
		m_completionEvent(INVALID_HANDLE_VALUE)
	{
		InternalCreate();
	}

	TimerQueue::TimerQueue(const HANDLE completionEvent)
	:	m_timer(nullptr),
		m_completionEvent(completionEvent)
	{
		if (m_completionEvent == nullptr)
			throw std::invalid_argument(__FUNCSIG__ ": completionEvent cannot be nullptr");
		if (m_completionEvent == INVALID_HANDLE_VALUE)
			throw std::invalid_argument(__FUNCSIG__ ": completionEvent cannot be INVALID_HANDLE_VALUE");
		InternalCreate();
	}

	TimerQueue::TimerQueue(const bool waitForAllCallbacks)
	:	m_timer(nullptr),
		m_completionEvent(waitForAllCallbacks ? INVALID_HANDLE_VALUE : nullptr)
	{
		InternalCreate();
	}


	TimerQueue::TimerQueue(TimerQueue&& other) noexcept
	{
		Move(other);
	}

	TimerQueue& TimerQueue::operator=(TimerQueue&& other) noexcept
	{
		Move(other);
		return *this;
	}

	void TimerQueue::Move(TimerQueue& other) noexcept
	{
		try
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
	}

	void TimerQueue::Close()
	{
		if (m_timer)
		{
			//https://docs.microsoft.com/en-us/windows/win32/api/threadpoollegacyapiset/nf-threadpoollegacyapiset-deletetimerqueueex
			if (!DeleteTimerQueueEx(m_timer, m_completionEvent))
				throw Error::Win32Error(__FUNCSIG__": DeleteTimerQueueEx() failed", GetLastError());
			m_timer = nullptr;
		}
	}

	bool TimerQueue::Close(const std::nothrow_t) noexcept
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

	HANDLE TimerQueue::GetHandle() const noexcept
	{
		return m_timer;
	}

	void TimerQueue::InternalCreate()
	{
		//https://docs.microsoft.com/en-us/windows/win32/api/threadpoollegacyapiset/nf-threadpoollegacyapiset-createtimerqueue
		m_timer = CreateTimerQueue();
		if (m_timer == nullptr)
			throw Error::Win32Error(__FUNCSIG__": CreateTimerQueue() failed", GetLastError());
	}
}
