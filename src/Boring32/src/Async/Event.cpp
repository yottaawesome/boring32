#include "pch.hpp"
#include <stdexcept>
#include "include/Error/Win32Error.hpp"
#include "include/Async/Event.hpp"

namespace Boring32::Async
{
	Event::~Event()
	{
		Close();
	}
	
	void Event::Close()
	{
		m_event = nullptr;
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
		m_event = CreateEventW(
			nullptr,			// security attributes
			m_isManualReset,	// manual reset event
			isSignaled,			// is initially signalled
			m_name != L""		// name
				? m_name.c_str()
				: nullptr);
		if (m_event == nullptr)
			throw Error::Win32Error("Failed to create or open event", GetLastError());
		m_event.SetInheritability(isInheritable);
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
			throw Error::Win32Error("Failed to create or open event", GetLastError());
	}

	Event::Event(const Event& other) 
	{ 
		Copy(other);
	}

	Event& Event::operator=(const Event& other)
	{
		Copy(other);
		return *this;
	}

	void Event::Copy(const Event& other)
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

	Event& Event::operator=(Event&& other) noexcept
	{
		Close();
		Move(other);
		return *this;
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
			if(ResetEvent(m_event.GetHandle()) == false)
				throw Error::Win32Error("ResetEvent failed", GetLastError());
	}
	
	HANDLE Event::GetHandle() const
	{
		return m_event.GetHandle();
	}

	void Event::WaitOnEvent()
	{
		if (m_event == nullptr)
			throw std::runtime_error("No Event to wait on");

		DWORD status = WaitForSingleObject(m_event.GetHandle(), INFINITE);
		if (status == WAIT_FAILED)
			throw Error::Win32Error("WaitForSingleObject failed", GetLastError());
		if (status == WAIT_ABANDONED)
			throw std::runtime_error("The wait was abandoned");
	}

	bool Event::WaitOnEvent(const DWORD millis, const bool alertable)
	{
		if (m_event == nullptr)
			throw std::runtime_error("No Event to wait on");

		DWORD status = WaitForSingleObject(m_event.GetHandle(), millis);
		if (status == WAIT_OBJECT_0)
			return true;
		if (status == WAIT_TIMEOUT)
			return false;
		if (status == WAIT_FAILED)
			throw Error::Win32Error("WaitForSingleObject failed", GetLastError());
		if (status == WAIT_ABANDONED)
			throw std::runtime_error("The wait was abandoned");
		return false;
	}
	
	HANDLE Event::Detach()
	{
		return m_event.Detach();
	}

	void Event::Signal()
	{
		if (m_event == nullptr)
			throw std::runtime_error("No Event to signal");
		if (SetEvent(m_event.GetHandle()) == false)
			throw Error::Win32Error("Failed to signal event", GetLastError());
	}
}
