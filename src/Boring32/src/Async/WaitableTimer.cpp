#include "pch.hpp"
#include <stdexcept>
#include "include/Async/WaitableTimer.hpp"

namespace Boring32::Async
{
	WaitableTimer::~WaitableTimer() 
	{
		Close();
	}

	void WaitableTimer::Close()
	{
		if (m_handle != nullptr)
		{
			CancelTimer();
			m_handle = nullptr;
		}
	}

	WaitableTimer::WaitableTimer()
	:	m_name(L""),
		m_isManualReset(false)
	{ }

	WaitableTimer::WaitableTimer(
		const std::wstring& name, 
		const bool isInheritable, 
		const bool isManualReset
	)
	:	m_name(name),
		m_isManualReset(isManualReset)
	{
		m_handle = CreateWaitableTimerW(
			nullptr,
			isManualReset,
			m_name == L""
				? nullptr
				: m_name.c_str()
		);
		if (m_handle == nullptr)
			throw std::runtime_error("Failed to create waitable timer");
		m_handle.SetInheritability(isInheritable);
	}

	WaitableTimer::WaitableTimer(
		const std::wstring& name, 
		const bool isInheritable, 
		const bool isManualReset, 
		const DWORD desiredAccess
	)
	:	m_name(name),
		m_isManualReset(isManualReset)
	{
		//TIMER_ALL_ACCESS
		m_handle = OpenWaitableTimerW(desiredAccess, isInheritable, name.c_str());
		if (m_handle == nullptr)
			throw std::runtime_error("Failed to open waitable timer");
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
	
	void WaitableTimer::SetTimerInNanos(const int64_t hundredNanosecondIntervals, const UINT period)
	{
		if (m_handle == nullptr)
			throw std::runtime_error("Timer handle is null");
		InternalSetTimer(hundredNanosecondIntervals, period);
	}

	void WaitableTimer::SetTimerInMillis(const int64_t ms, const UINT period)
	{
		if (m_handle == nullptr)
			throw std::runtime_error("Timer handle is null");
		InternalSetTimer(ms * 10000, period);
	}

	void WaitableTimer::InternalSetTimer(const int64_t time, const UINT period)
	{
		LARGE_INTEGER liDueTime;
		liDueTime.QuadPart = time;
		bool succeeded = SetWaitableTimer(
			m_handle.GetHandle(), 
			&liDueTime, 
			period,
			nullptr, 
			nullptr, 
			false
		);
		if (succeeded == false)
			throw std::runtime_error("Failed to set timer");
	}

	bool WaitableTimer::WaitOnTimer(const DWORD millis)
	{
		if (m_handle == nullptr)
			throw std::runtime_error("No timer to wait on");
		DWORD status = WaitForSingleObject(m_handle.GetHandle(), millis);
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

	bool WaitableTimer::CancelTimer()
	{
		if (m_handle == nullptr)
			throw std::runtime_error("No timer to cancel");
		return CancelWaitableTimer(m_handle.GetHandle());
	}

	bool WaitableTimer::IsManualReset() const
	{
		return m_isManualReset;
	}

	std::wstring WaitableTimer::GetName() const
	{
		return m_name;
	}

	HANDLE WaitableTimer::GetHandle() const
	{
		return m_handle.GetHandle();
	}
}