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

	WaitableTimer::WaitableTimer(const bool createNew, const std::wstring& name, const bool isInheritable, const bool isManualReset)
	:	m_name(name),
		m_isManualReset(isManualReset)
	{
		SECURITY_ATTRIBUTES sa{ 0 };
		sa.nLength = sizeof(sa);
		sa.bInheritHandle = isInheritable;
		if (createNew)
		{
			m_handle = CreateWaitableTimer(
				&sa,
				isManualReset,
				m_name == L""
					? nullptr
					: m_name.c_str()
			);
		}
		else
		{
			m_handle = OpenWaitableTimer(TIMER_ALL_ACCESS, isInheritable, name.c_str());
		}

		if (m_handle == nullptr)
			throw std::runtime_error("Failed to create waitable timer");
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

	bool WaitableTimer::IsManualReset()
	{
		return m_isManualReset;
	}

	std::wstring WaitableTimer::GetName()
	{
		return m_name;
	}

	HANDLE WaitableTimer::GetHandle()
	{
		return m_handle.GetHandle();
	}
}