#include "pch.hpp"
#include <iostream>
#include "include/Error/Win32Error.hpp"
#include "include/Async/TimerQueue.hpp"

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

	TimerQueue::TimerQueue(HANDLE completionEvent)
	:	m_timer(nullptr),
		m_completionEvent(completionEvent)
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
			bool succeeded = DeleteTimerQueueEx(m_timer, m_completionEvent);
			if (succeeded == false)
				throw Error::Win32Error("TimerQueue::Close(): DeleteTimerQueueEx() failed", GetLastError());
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

	HANDLE TimerQueue::GetHandle()
	{
		return m_timer;
	}

	void TimerQueue::InternalCreate()
	{
		//https://docs.microsoft.com/en-us/windows/win32/api/threadpoollegacyapiset/nf-threadpoollegacyapiset-createtimerqueue
		m_timer = CreateTimerQueue();
		if (m_timer == nullptr)
			throw Error::Win32Error("TimerQueue::InternalCreate(): CreateTimerQueue() failed", GetLastError());
	}
}
