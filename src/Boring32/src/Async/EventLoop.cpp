#include "pch.hpp"
#include "include/Async/EventLoop.hpp"
#include "include/Error/Error.hpp"

namespace Boring32::Async
{
	EventLoop::~EventLoop() 
	{ 
		Close(); 
	}
	
	EventLoop::EventLoop() {}

	EventLoop::EventLoop(std::map<HANDLE, std::function<void(EventLoop&)>&> mapOfEvents)
	:	m_mapOfEvents(std::move(mapOfEvents))
	{ 
		RebuildEvents();
	}

	void EventLoop::Close() 
	{
		m_mapOfEvents.clear();
	}
	
	bool EventLoop::WaitOn(const DWORD millis, const bool waitAll)
	{
		if (m_events.size() == 0)
			throw std::runtime_error("EventLoop::WaitOn(): m_events is empty");

		DWORD result = WaitForMultipleObjectsEx(m_events.size(), &m_events[0], waitAll, millis, true);
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
			for (int i = 0; i < m_events.size(); i++)
				m_mapOfEvents[m_events[i]](*this);
		}
		else
		{
			// If we didn't wait for all events to fire, then we need to 
			// to fire the relevant event's function, in addition to any
			// other events that are active. This is because
			// WaitForMultipleObjectsEx() only returns the lowest index of 
			// the fire events, and there could be potentially more.
			m_mapOfEvents[m_events[result]](*this);
			for (int i = result - WAIT_OBJECT_0; i < m_events.size(); i++)
				if (WaitForSingleObject(m_events[i], 0) == (WAIT_OBJECT_0 - i))
					m_mapOfEvents[m_events[i]](*this);
		}

		return true;
	}
	
	void EventLoop::Set(std::map<HANDLE, std::function<void(EventLoop&)>&> mapOfEvents)
	{
		m_mapOfEvents = std::move(mapOfEvents);
		RebuildEvents();
	}

	void EventLoop::Set(HANDLE handle, std::function<void(EventLoop&)>& function)
	{
		m_mapOfEvents[handle] = function;
	}

	void EventLoop::RebuildEvents()
	{
		m_events.clear();
		for (const auto& [handle, function] : m_mapOfEvents)
			m_events.push_back(handle);
	}
}