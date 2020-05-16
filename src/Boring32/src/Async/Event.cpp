#include "pch.hpp"
#include <stdexcept>
#include "include/Async/Event.hpp"

namespace Win32Utils::Async
{
	Event::~Event()
	{
		Close();
	}
	
	void Event::Close()
	{
		if (m_event)
		{
			CloseHandle(m_event);
			m_event = nullptr;
		}
	}

	Event::Event()
	:	m_event(nullptr),
		m_isInheritable(false),
		m_isManualReset(false),
		m_isSignaled(false),
		m_name(L"")
	{ }

	Event::Event(
		const bool isInheritable,
		const bool manualReset,
		const bool isSignaled,
		const std::wstring& name
	)
	:	m_event(nullptr),
		m_isInheritable(isInheritable),
		m_isManualReset(manualReset),
		m_isSignaled(isSignaled),
		m_name(name)
	{
		SECURITY_ATTRIBUTES sp{ 0 };
		sp.nLength = sizeof(sp);
		sp.bInheritHandle = true;

		m_event = CreateEvent(
			&sp,				// security attributes
			m_isManualReset,    // manual reset event
			m_isSignaled,		// is initially signalled
			m_name != L""		// name
				? m_name.c_str() 
				: nullptr);   
	}

	Event::Event(const Event& other) 
	{ 
		Duplicate(other);
	}

	void Event::operator=(const Event& other)
	{
		Close();
		Duplicate(other);
	}

	void Event::Duplicate(const Event& other)
	{
		other.m_isInheritable;
		other.m_isManualReset;
		other.m_isSignaled;
		other.m_name;
		if (other.m_event)
		{
			DuplicateHandle(
				GetCurrentProcess(),
				other.m_event,
				GetCurrentProcess(),
				&m_event,
				0,
				m_isInheritable,
				DUPLICATE_SAME_ACCESS
			);
			if (m_event == nullptr)
				throw std::runtime_error("Failed to duplicate event handle");
		}
	}

	Event::Event(Event&& other) noexcept
	{
		Move(other);
	}

	void Event::operator=(Event& other) noexcept
	{
		Close();
		Move(other);
	}

	void Event::Move(Event& other) noexcept
	{
		m_isInheritable=other.m_isInheritable;
		m_isManualReset = other.m_isManualReset;
		m_isSignaled = other.m_isSignaled;
		m_name = other.m_name;
		m_event = other.m_event;
		other.m_event = nullptr;
	}

	void Event::Reset()
	{
		if (m_isManualReset && m_event)
			ResetEvent(m_event);
	}
	
	HANDLE Event::GetHandle()
	{
		return m_event;
	}

	void Event::WaitOnEvent()
	{
		if (m_event == nullptr)
			throw std::runtime_error("Event not created to wait on");

		DWORD status = WaitForSingleObject(m_event, INFINITE);
		if (status == WAIT_FAILED)
			throw std::runtime_error("WaitForSingleObject failed");
		if (status == WAIT_ABANDONED)
			throw std::runtime_error("The wait was abandoned");
	}

	bool Event::WaitOnEvent(const DWORD millis)
	{
		if (m_event == nullptr)
			throw std::runtime_error("Event not created to wait on");

		DWORD status = WaitForSingleObject(m_event, millis);
		if (status == WAIT_OBJECT_0)
			return true;
		if (status == WAIT_TIMEOUT)
			return false;
		if (status == WAIT_FAILED)
			throw std::runtime_error("WaitForSingleObject failed");
		if (status == WAIT_ABANDONED)
			throw std::runtime_error("The wait was abandoned");
		return false;
	}
}
