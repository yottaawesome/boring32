#include "pch.hpp"
#include <stdexcept>
#include "include/Error/Win32Exception.hpp"
#include "include/Async/Event.hpp"

namespace Boring32::Async
{
	Event::~Event()
	{
		Close();
	}
	
	void Event::Close()
	{
		if (m_event != nullptr)
		{
			m_event.Close();
			m_event = nullptr;
		}
	}

	Event::Event()
	:	m_event(nullptr),
		m_isManualReset(false),
		m_name(L""),
		m_createEventOnTrue(false),
		m_access(0)
	{ }

	Event::Event(
		const bool isInheritable,
		const bool manualReset,
		const bool isSignaled,
		const std::wstring& name
	)
	:	m_event(nullptr),
		m_isManualReset(manualReset),
		m_name(name),
		m_createEventOnTrue(true),
		m_access(EVENT_ALL_ACCESS)
	{
		SECURITY_ATTRIBUTES sp{ 0 };
		sp.nLength = sizeof(sp);
		sp.bInheritHandle = isInheritable;

		m_event = CreateEventW(
			&sp,				// security attributes
			m_isManualReset,    // manual reset event
			isSignaled,		// is initially signalled
			m_name != L""		// name
				? m_name.c_str()
				: nullptr);
		if (m_event == nullptr)
			throw Error::Win32Exception("Failed to create or open event", GetLastError());
	}

	Event::Event(
		const bool isInheritable,
		const bool manualReset,
		const std::wstring& name,
		const DWORD desiredAccess
	)
	:	m_event(nullptr),
		m_isManualReset(manualReset),
		m_name(name),
		m_createEventOnTrue(false),
		m_access(desiredAccess)
	{
		m_event = OpenEventW(m_access, isInheritable, m_name.c_str());
		if (m_event == nullptr)
			throw Error::Win32Exception("Failed to create or open event", GetLastError());
	}

	Event::Event(const Event& other) 
	{ 
		Duplicate(other);
	}

	void Event::operator=(const Event& other)
	{
		Duplicate(other);
	}

	void Event::Duplicate(const Event& other)
	{
		Close();
		m_isManualReset = other.m_isManualReset;
		m_name = other.m_name;
		m_event = other.m_event;
	}

	Event::Event(Event&& other) noexcept
	{
		Move(other);
	}

	void Event::operator=(Event&& other) noexcept
	{
		Close();
		Move(other);
	}

	void Event::Move(Event& other) noexcept
	{
		m_isManualReset = other.m_isManualReset;
		m_name = std::move(other.m_name);
		m_event = std::move(other.m_event);
		other.m_event = nullptr;
	}

	void Event::Reset()
	{
		if (m_isManualReset && m_event != nullptr)
			ResetEvent(m_event.GetHandle());
	}
	
	HANDLE Event::GetHandle()
	{
		return m_event.GetHandle();
	}

	void Event::WaitOnEvent()
	{
		if (m_event == nullptr)
			throw std::runtime_error("No Event to wait on");

		DWORD status = WaitForSingleObject(m_event.GetHandle(), INFINITE);
		if (status == WAIT_FAILED)
			throw std::runtime_error("WaitForSingleObject failed");
		if (status == WAIT_ABANDONED)
			throw std::runtime_error("The wait was abandoned");
	}

	bool Event::WaitOnEvent(const DWORD millis)
	{
		if (m_event == nullptr)
			throw std::runtime_error("No Event to wait on");

		DWORD status = WaitForSingleObject(m_event.GetHandle(), millis);
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

	void Event::Signal()
	{
		if (m_event == nullptr)
			throw std::runtime_error("No Event to signal");
		if (SetEvent(m_event.GetHandle()) == false)
			throw Error::Win32Exception("Failed to signal event", GetLastError());
	}
}
