module;

#include <source_location>

module boring32.async:event;
import boring32.error;
import <stdexcept>;
import <iostream>;
import <format>;

namespace Boring32::Async
{	
	void Event::Close()
	{
		m_event = nullptr;
	}

	Event::Event(
		const bool isInheritable,
		const bool manualReset,
		const bool isSignaled
	)
	:	m_isManualReset(manualReset)
	{
		InternalCreate(isSignaled, isInheritable);
	}

	Event::Event(
		const bool isInheritable,
		const bool manualReset,
		const bool isSignaled,
		std::wstring name
	)
	:	m_isManualReset(manualReset),
		m_name(std::move(name))
	{
		InternalCreate(isSignaled, isInheritable);
	}

	Event::Event(
		const bool isInheritable,
		const bool manualReset,
		std::wstring name,
		const DWORD desiredAccess
	)
	:	m_isManualReset(manualReset),
		m_name(std::move(name))
	{
		m_event = OpenEventW(desiredAccess, isInheritable, m_name.c_str());
		if (!m_event)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("Failed to create or open event", lastError);
		}
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

	Event::operator HANDLE() const noexcept
	{
		return *m_event;
	}

	Event::operator bool() const noexcept
	{
		return m_event != nullptr;
	}

	void Event::Move(Event& other) noexcept
	{
		m_isManualReset = other.m_isManualReset;
		m_name = std::move(other.m_name);
		m_event = std::move(other.m_event);
	}

	void Event::Reset()
	{
		if (!m_isManualReset)
			return;
		if (!m_event)
			return;
		if (!ResetEvent(m_event.GetHandle()))
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("ResetEvent() failed", lastError);
		}
	}

	bool Event::Reset(std::nothrow_t) noexcept try
	{
		Reset();
		return true;
	}
	catch (const std::exception& ex)
	{
		std::wcerr << std::format(
			"{}: Reset() failed: {}", 
			std::source_location::current().function_name(),
			ex.what()
		).c_str();
		return false;
	}

	HANDLE Event::GetHandle() const noexcept
	{
		return m_event.GetHandle();
	}

	void Event::WaitOnEvent() const
	{
		if (!m_event)
			throw Error::Boring32Error("No Event to wait on");

		const DWORD status = WaitForSingleObject(m_event.GetHandle(), INFINITE);
		if (status == WAIT_FAILED)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("WaitForSingleObject failed", lastError);
		}
		if (status == WAIT_ABANDONED)
			throw Error::Boring32Error("The wait was abandoned");
	}

	bool Event::WaitOnEvent(
		const DWORD millis, 
		const bool alertable
	) const
	{
		if (!m_event)
			throw Error::Boring32Error("No Event to wait on");

		const DWORD status = WaitForSingleObjectEx(m_event.GetHandle(), millis, alertable);
		if (status == WAIT_OBJECT_0)
			return true;
		if (status == WAIT_TIMEOUT)
			return false;
		if (status == WAIT_FAILED)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("WaitForSingleObject() failed", lastError);
		}
		if (status == WAIT_ABANDONED)
			throw Error::Boring32Error("The wait was abandoned");
		return false;
	}

	bool Event::WaitOnEvent(
		const DWORD millis, 
		const bool alertable, 
		std::nothrow_t
	) const noexcept try
	{
		//https://codeyarns.com/tech/2018-08-22-how-to-get-function-name-in-c.html
		WaitOnEvent();
		return true;
	}
	catch (const std::exception& ex)
	{
		std::wcerr << std::format(
			"{}: WaitOnEvent() failed: {}", 
			std::source_location::current().function_name(),
			ex.what()
		).c_str();
		return false;
	}

	HANDLE Event::Detach() noexcept
	{
		return m_event.Detach();
	}

	void Event::Signal()
	{
		if (!m_event)
			throw Error::Boring32Error("No Event to signal");
		if (!SetEvent(m_event.GetHandle()))
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("Failed to signal event", lastError);
		}
	}

	bool Event::Signal(std::nothrow_t) noexcept try
	{
		Signal();
		return true;
	}
	catch (const std::exception& ex)
	{
		std::wcerr << std::format(
			"{}: Signal() failed: {}", 
			std::source_location::current().function_name(),
			ex.what()
		).c_str();
		return false;
	}

	const std::wstring& Event::GetName() const noexcept
	{
		return m_name;
	}
	
	bool Event::IsManualReset() const noexcept
	{
		return m_isManualReset;
	}

	void Event::InternalCreate(const bool isSignaled, const bool isInheritable)
	{
		m_event = CreateEventW(
			nullptr,			// security attributes
			m_isManualReset,	// manual reset event
			isSignaled,			// is initially signalled
			m_name.empty() ? nullptr : m_name.c_str() // name
		);
		if (!m_event)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("Failed to create or open event", lastError);
		}
		m_event.SetInheritability(isInheritable);
	}
}
