module;

#include <stdexcept>
#include <string>
#include <iostream>
#include <format>
#include <source_location>
#include <Windows.h>

module boring32.async:waitabletimer;
import boring32.error;

//https://docs.microsoft.com/en-us/windows/win32/sync/using-a-waitable-timer-with-an-asynchronous-procedure-call
namespace Boring32::Async
{
	WaitableTimer::~WaitableTimer() 
	{
		Close();
	}

	void WaitableTimer::Close()
	{
		m_handle = nullptr;
	}

	WaitableTimer::WaitableTimer()
	:	m_isManualReset(false)
	{ }

	WaitableTimer::WaitableTimer(
		const bool isInheritable,
		const bool isManualReset
	) : m_isManualReset(isManualReset)
	{ 
		InternalCreate(isInheritable);
	}

	WaitableTimer::WaitableTimer(
		std::wstring name, 
		const bool isInheritable, 
		const bool isManualReset
	)
	:	m_name(std::move(name)),
		m_isManualReset(isManualReset)
	{
		if (m_name.empty())
			throw Error::Boring32Error(std::source_location::current(), "Name cannot be empty on WaitableTimer");

		InternalCreate(isInheritable);
	}

	WaitableTimer::WaitableTimer(
		std::wstring name, 
		const bool isInheritable, 
		const bool isManualReset, 
		const DWORD desiredAccess
	)
	:	m_name(std::move(name)),
		m_isManualReset(isManualReset)
	{
		if (m_name.empty())
			throw Error::Boring32Error(std::source_location::current(), "Name cannot be empty on WaitableTimer");

		//TIMER_ALL_ACCESS
		//https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-openwaitabletimerw
		m_handle = OpenWaitableTimerW(desiredAccess, isInheritable, m_name.c_str());
		if (!m_handle)
			throw Error::Win32Error(std::source_location::current(), "failed to open waitable timer", GetLastError());
	}

	// Copy constructor
	WaitableTimer::WaitableTimer(const WaitableTimer& other)
	{
		Copy(other);
	}
	
	void WaitableTimer::operator=(const WaitableTimer& other)
	{
		Copy(other);
	}

	void WaitableTimer::Copy(const WaitableTimer& other)
	{
		Close();
		m_name = other.m_name;
		m_handle = other.m_handle;
		m_isManualReset = other.m_isManualReset;
	}
	
	// Move constructor
	WaitableTimer::WaitableTimer(WaitableTimer&& other) noexcept
	{
		Move(other);
	}
	
	void WaitableTimer::operator=(WaitableTimer&& other) noexcept
	{
		Move(other);
	}
	
	void WaitableTimer::Move(WaitableTimer& other) noexcept
	{
		Close();
		m_name = std::move(other.m_name);
		m_handle = std::move(other.m_handle);
		m_isManualReset = other.m_isManualReset;
	}
	
	void WaitableTimer::SetTimerInNanos(
		const int64_t hundredNanosecondIntervals, 
		const UINT period,
		const PTIMERAPCROUTINE callback,
		void* param
	)
	{
		if (!m_handle)
			throw Error::Boring32Error(std::source_location::current(), "Timer handle is null");
		InternalSetTimer(hundredNanosecondIntervals, period, callback, param);
	}

	bool WaitableTimer::SetTimerInNanos(
		const int64_t hundedNsIntervals,
		const UINT period,
		const PTIMERAPCROUTINE callback,
		void* param,
		const std::nothrow_t&
	) noexcept try
	{
		SetTimerInNanos(hundedNsIntervals, period, callback, param);
		return true;
	}
	catch (const std::exception& ex)
	{
		std::wcerr << std::format("{}: SetTimerInNanos() failed: {}\n", __FUNCSIG__, ex.what()).c_str();
		return false;
	}

	void WaitableTimer::SetTimerInMillis(
		const int64_t ms, 
		const UINT period,
		const PTIMERAPCROUTINE callback,
		void* param
	)
	{
		if (!m_handle)
			throw Error::Boring32Error(std::source_location::current(), "Timer handle is null");
		InternalSetTimer(ms * 10000, period, callback, param);
	}

	bool WaitableTimer::SetTimerInMillis(
		const int64_t milliseconds,
		const UINT period,
		const PTIMERAPCROUTINE callback,
		void* param,
		const std::nothrow_t&
	) noexcept try
	{
		SetTimerInMillis(milliseconds, period, callback, param);
		return true;
	}
	catch (const std::exception& ex)
	{
		std::wcerr << std::format("{}: SetTimerInMillis() failed: {}\n", __FUNCSIG__, ex.what()).c_str();
		return false;
	}

	void WaitableTimer::InternalSetTimer(
		const int64_t time, 
		const UINT period,
		const PTIMERAPCROUTINE callback,
		void* param
	)
	{
		LARGE_INTEGER liDueTime;
		liDueTime.QuadPart = time;
		//https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-setwaitabletimer
		const bool succeeded = SetWaitableTimer(
			m_handle.GetHandle(),
			&liDueTime,
			period,
			callback,
			param,
			false
		);
		if (!succeeded)
			throw Error::Win32Error(std::source_location::current(), "Failed to set timer", GetLastError());
	}

	bool WaitableTimer::WaitOnTimer(const DWORD millis)
	{
		if (!m_handle)
			throw Error::Boring32Error(std::source_location::current(), "Timer handle is null");

		const DWORD status = WaitForSingleObject(m_handle.GetHandle(), millis);
		switch (status)
		{
			case WAIT_OBJECT_0:
				return true;

			case WAIT_TIMEOUT:
				return false;

			case WAIT_ABANDONED:
				throw Error::Boring32Error(std::source_location::current(), "The wait was abandoned");

			case WAIT_FAILED:
			{
				const auto lastError = GetLastError();
				throw Error::Win32Error(std::source_location::current(), "WaitForSingleObject() failed", lastError);
			}
		}

		return false;
	}

	bool WaitableTimer::WaitOnTimer(
		const DWORD millis, 
		std::nothrow_t&
	) noexcept try
	{
		return WaitOnTimer(millis);
	}
	catch (const std::exception& ex)
	{
		std::wcerr << std::format("{}: WaitOnTimer() failed: {}\n", __FUNCSIG__, ex.what()).c_str();
		return false;
	}

	void WaitableTimer::CancelTimer()
	{
		if (!m_handle)
			throw Error::Boring32Error(std::source_location::current(), "Timer handle is null");

		if (!CancelWaitableTimer(m_handle.GetHandle()))
			throw Error::Win32Error(std::source_location::current(), "CancelWaitableTimer() failed", GetLastError());
	}

	bool WaitableTimer::CancelTimer(std::nothrow_t&) noexcept try
	{
		this->CancelTimer();
		return true;
	}
	catch (const std::exception& ex)
	{
		std::wcerr << std::format("{}: CancelTimer() failed: {}\n", __FUNCSIG__, ex.what()).c_str();
		return false;
	}

	bool WaitableTimer::IsManualReset() const noexcept
	{
		return m_isManualReset;
	}

	const std::wstring& WaitableTimer::GetName() const noexcept
	{
		return m_name;
	}

	HANDLE WaitableTimer::GetHandle() const noexcept
	{
		return m_handle.GetHandle();
	}
	
	void WaitableTimer::InternalCreate(const bool isInheritable)
	{
		//https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-createwaitabletimerw
		m_handle = CreateWaitableTimerW(
			nullptr,
			m_isManualReset,
			m_name.empty() ? nullptr : m_name.c_str()
		);
		if (!m_handle)
			throw Error::Win32Error(std::source_location::current(), "Failed to create waitable timer", GetLastError());
		m_handle.SetInheritability(isInheritable);
	}
}