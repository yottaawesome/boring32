module;

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <Windows.h>

module boring32.async.eventloop;
import boring32.error.win32error;
import boring32.async.criticalsectionlock;

namespace Boring32::Async
{
	EventLoop::~EventLoop() 
	{ 
		Close(); 
	}
	
	EventLoop::EventLoop() 
	{
		InitializeCriticalSection(&m_cs);
	}

	void EventLoop::Close()
	{
		m_handlers.clear();
		m_events.clear();
		DeleteCriticalSection(&m_cs);
	}
	
	bool EventLoop::WaitOn(const DWORD millis, const bool waitAll)
	{
		if (m_events.empty())
			throw std::runtime_error("EventLoop::WaitOn(): m_events is empty");

		CriticalSectionLock cs(m_cs);

		DWORD result = WaitForMultipleObjectsEx((DWORD)m_events.size(), &m_events[0], waitAll, millis, true);
		if (result == WAIT_FAILED)
			throw Error::Win32Error("EventLoop::WaitOn(): WaitForMultipleObjectsEx() failed", GetLastError());
		if (result == WAIT_TIMEOUT)
			return false;
		if (result >= WAIT_ABANDONED && result <= (WAIT_ABANDONED + m_events.size() - 1))
			throw std::runtime_error("EventLoop::WaitOn(): a wait object was abandoned");

		if (waitAll)
		{
			// If we waited for all events to fire, then we need to fire
			// all functions. This is because WaitForMultipleObjectsEx()
			// returns only the zero index.
			for (auto& handler : m_handlers)
				handler();
		}
		else
		{
			m_handlers.at(result)();
		}

		return true;
	}
	
	void EventLoop::On(HANDLE handle, std::function<void()> handler)
	{
		CriticalSectionLock cs(m_cs);
		m_events.push_back(handle);
		m_handlers.push_back(std::move(handler));
	}

	void EventLoop::Erase(HANDLE handle)
	{
		CriticalSectionLock cs(m_cs);
		std::vector<HANDLE>::iterator handlePosIterator = std::find_if(
			m_events.begin(), 
			m_events.end(), 
			[handle](const auto& elem) { return elem == handle; }
		);
		if (handlePosIterator != m_events.end())
		{
			std::ptrdiff_t iteratorDist = std::distance(m_events.begin(), handlePosIterator);
			m_events.erase(handlePosIterator);
			m_handlers.erase(m_handlers.begin() + iteratorDist);
		}
	}

	size_t EventLoop::Size() noexcept
	{
		CriticalSectionLock cs(m_cs);
		return m_events.size();
	}
}