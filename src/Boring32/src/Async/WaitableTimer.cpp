#include "pch.hpp"
#include <stdexcept>
#include "include/Error/Error.hpp"
#include "include/Async/WaitableTimer.hpp"

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
	:	m_name(L""),
		m_isManualReset(false)
	{ }

	WaitableTimer::WaitableTimer(
		std::wstring name, 
		const bool isInheritable, 
		const bool isManualReset
	)
	:	m_name(std::move(name)),
		m_isManualReset(isManualReset)
	{
		//https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-createwaitabletimerw
		m_handle = CreateWaitableTimerW(
			nullptr,
			isManualReset,
			m_name == L""
				? nullptr
				: m_name.c_str()
		);
		if (m_handle == nullptr)
			throw Error::Win32Error(__FUNCSIG__ ": failed to create waitable timer", GetLastError());
		m_handle.SetInheritability(isInheritable);
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
		//TIMER_ALL_ACCESS
		//https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-openwaitabletimerw
		m_handle = OpenWaitableTimerW(desiredAccess, isInheritable, m_name.c_str());
		if (m_handle == nullptr)
			throw Error::Win32Error(__FUNCSIG__ ": failed to open waitable timer", GetLastError());
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
		if (m_handle == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": timer handle is null");
		InternalSetTimer(hundredNanosecondIntervals, period, callback, param);
	}

	bool WaitableTimer::SetTimerInNanos(
		const int64_t hundedNsIntervals,
		const UINT period,
		const PTIMERAPCROUTINE callback,
		void* param,
		std::nothrow_t
	) noexcept
	{
		return Error::TryCatchLogToWCerr(
			[this, hundedNsIntervals, period, callback, param]
				{ SetTimerInNanos(hundedNsIntervals, period, callback, param); },
			__FUNCSIG__
		);
	}

	void WaitableTimer::SetTimerInMillis(
		const int64_t ms, 
		const UINT period,
		const PTIMERAPCROUTINE callback,
		void* param
	)
	{
		if (m_handle == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": timer handle is null");
		InternalSetTimer(ms * 10000, period, callback, param);
	}

	bool WaitableTimer::SetTimerInMillis(
		const int64_t milliseconds,
		const UINT period,
		const PTIMERAPCROUTINE callback,
		void* param,
		std::nothrow_t
	) noexcept
	{
		return Error::TryCatchLogToWCerr(
			[this, milliseconds, period, callback, param]
				{ SetTimerInMillis(milliseconds, period, callback, param); },
			__FUNCSIG__
		);
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
		bool succeeded = SetWaitableTimer(
			m_handle.GetHandle(),
			&liDueTime,
			period,
			callback,
			param,
			false
		);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__ ": failed to set timer", GetLastError());
	}

	bool WaitableTimer::WaitOnTimer(const DWORD millis)
	{
		if (m_handle == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": no timer to wait on");
		DWORD status = WaitForSingleObject(m_handle.GetHandle(), millis);
		if (status == WAIT_OBJECT_0)
			return true;
		if (status == WAIT_TIMEOUT)
			return false;
		if (status == WAIT_FAILED)
			throw std::runtime_error(__FUNCSIG__ ": WaitForSingleObject failed");
		if (status == WAIT_ABANDONED)
			throw std::runtime_error(__FUNCSIG__ ": the wait was abandoned");
		return false;
	}

	bool WaitableTimer::WaitOnTimer(const DWORD millis, std::nothrow_t) noexcept
	{
		return Error::TryCatchLogToWCerr(
			[this, millis] { WaitOnTimer(millis); },
			__FUNCSIG__
		);
	}

	void WaitableTimer::CancelTimer()
	{
		if (m_handle == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": m_handle is nullptr");
		bool succeeded = CancelWaitableTimer(m_handle.GetHandle());
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__, GetLastError());
	}

	bool WaitableTimer::CancelTimer(std::nothrow_t) noexcept
	{
		return Error::TryCatchLogToWCerr([this] { this->CancelTimer(); }, __FUNCSIG__);
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
}