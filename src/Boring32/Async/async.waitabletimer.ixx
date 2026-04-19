export module boring32:async.waitabletimer;
import std;
import :win32;
import :error;
import :raii;
import :async.functions;
import :concepts;

//https://docs.microsoft.com/en-us/windows/win32/sync/using-a-waitable-timer-with-an-asynchronous-procedure-call
export namespace Boring32::Async
{
	struct WaitableTimer final
	{
		~WaitableTimer()
		{
			Close();
		}

		WaitableTimer() = default;

		WaitableTimer(const WaitableTimer& other)
		{
			Copy(other);
		}

		WaitableTimer(WaitableTimer&& other) noexcept
		{
			Move(other);
		}

		WaitableTimer(bool isInheritable, bool isManualReset) 
			: m_isManualReset(isManualReset)
		{
			InternalCreate(isInheritable);
		}

		WaitableTimer(std::wstring name, bool isInheritable, bool isManualReset) 
			: m_name(std::move(name)),
			m_isManualReset(isManualReset)
		{
			if (m_name.empty())
				throw Error::Boring32Error("Name cannot be empty on WaitableTimer");

			InternalCreate(isInheritable);
		}

		WaitableTimer(std::wstring name, bool isInheritable, bool isManualReset, Win32::DWORD desiredAccess) 
			: m_name(std::move(name)),
			m_isManualReset(isManualReset)
		{
			if (m_name.empty())
				throw Error::Boring32Error("Name cannot be empty on WaitableTimer");

			//TIMER_ALL_ACCESS
			//https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-openwaitabletimerw
			m_handle = Win32::OpenWaitableTimerW(desiredAccess, isInheritable, m_name.c_str());
			if (not m_handle)
				throw Error::Win32Error{Win32::GetLastError(), "Failed to open waitable timer"};
		}

		auto operator=(const WaitableTimer& other) -> WaitableTimer&
		{
			Copy(other);
			return *this;
		}

		auto operator=(WaitableTimer&& other) noexcept -> WaitableTimer&
		{
			Move(other);
			return *this;
		}

		///	Set this timer using 100-nanosecond intervals.
		///	"hundredNanosecondIntervals"
		///	The time after which the state of the timer is to be set to signaled, in 100 nanosecond intervals.
		///	Positive values indicate absolute time. Be sure to use a UTC-based absolute time, 
		///	as the system uses UTC-based time internally. Negative values indicate relative time. 
		///	The actual timer accuracy depends on the capability of your hardware.
		/// period
		///	The period of the timer, in milliseconds. If period is zero,
		///	the timer is signaled once, else if it is greater than zero, the
		///	timer is periodic. A periodic timer automatically reactivates
		///	each time the period elapses, until the timer is canceled.
		auto SetTimerInNanos(
			const int64_t hundredNanosecondIntervals, 
			const Win32::UINT period,
			const Win32::PTIMERAPCROUTINE callback,
			void* param
		) -> void
		{
			if (not m_handle)
				throw Error::Boring32Error("Timer handle is null");
			InternalSetTimer(hundredNanosecondIntervals, period, callback, param);
		}

		auto SetTimerInNanos(
			const int64_t hundedNsIntervals,
			const Win32::UINT period,
			const Win32::PTIMERAPCROUTINE callback,
			void* param,
			const std::nothrow_t&
		) noexcept -> bool
		try
		{
			SetTimerInNanos(hundedNsIntervals, period, callback, param);
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}

		auto SetTimerInMillis(
			const int64_t ms,
			const Win32::UINT period,
			const Win32::PTIMERAPCROUTINE callback,
			void* param
		) -> void
		{
			if (not m_handle)
				throw Error::Boring32Error("Timer handle is null");
			InternalSetTimer(ms * 10000, period, callback, param);
		}

		auto SetTimerInMillis(
			int64_t milliseconds,
			Win32::UINT period,
			const Win32::PTIMERAPCROUTINE callback,
			void* param,
			std::nothrow_t
		) noexcept -> bool
		try
		{
			SetTimerInMillis(milliseconds, period, callback, param);
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}

		auto WaitOnTimer(Concepts::Duration auto time, bool alertable) -> bool
		{
			if (not m_handle)
				throw Error::Boring32Error("Timer handle is null");
			return WaitFor(m_handle, time, alertable);
		}

		template<typename T>
		auto WaitOnTimer(Concepts::Duration auto time, bool alertable, std::nothrow_t) -> bool
		try
		{
			if (not m_handle)
				throw Error::Boring32Error("Timer handle is null");
			return WaitFor(m_handle, time, alertable);
		}
		catch (const std::exception& ex)
		{
			return false;
		}

		auto WaitOnTimer(const Win32::DWORD millis, const bool alertable) -> bool
		{
			if (not m_handle)
				throw Error::Boring32Error("Timer handle is null");
			return WaitFor(m_handle, millis, alertable) == Win32::WaitResult::Success;
		}

		auto WaitOnTimer(Win32::DWORD millis, bool alertable, std::nothrow_t) noexcept -> bool
		try
		{
			return WaitOnTimer(millis, alertable);
		}
		catch (const std::exception&)
		{
			return false;
		}

		auto CancelTimer() -> void
		{
			if (not m_handle)
				throw Error::Boring32Error("Timer handle is null");
			if (not Win32::CancelWaitableTimer(m_handle.GetHandle()))
				throw Error::Win32Error{Win32::GetLastError(), "CancelWaitableTimer() failed"};
		}

		auto CancelTimer(std::nothrow_t) noexcept -> bool
		try
		{
			this->CancelTimer();
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}

		auto IsManualReset() const noexcept -> bool
		{
			return m_isManualReset;
		}

		auto GetName() const noexcept -> const std::wstring&
		{
			return m_name;
		}

		///	Returns the underlying HANDLE for this timer, or null
		///	if no timer has been created.
		auto GetHandle() const noexcept -> Win32::HANDLE
		{
			return m_handle.GetHandle();
		}

		///	Frees all native resources associated with this timer.
		auto Close() -> void
		{
			m_handle = nullptr;
		}

		protected:
		auto Move(WaitableTimer& other) noexcept -> void
		{
			Close();
			m_name = std::move(other.m_name);
			m_handle = std::move(other.m_handle);
			m_isManualReset = other.m_isManualReset;
		}

		auto Copy(const WaitableTimer& other) -> void
		{
			Close();
			m_name = other.m_name;
			m_handle = other.m_handle;
			m_isManualReset = other.m_isManualReset;
		}

		auto InternalCreate(const bool isInheritable) -> void
		{
			//https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-createwaitabletimerw
			m_handle = Win32::CreateWaitableTimerW(
				nullptr,
				m_isManualReset,
				m_name.empty() ? nullptr : m_name.c_str()
			);
			if (not m_handle)
				throw Error::Win32Error{Win32::GetLastError(), "Failed to create waitable timer"};
			m_handle.SetInheritability(isInheritable);
		}

		auto InternalSetTimer(
			const int64_t time, 
			const Win32::UINT period,
			const Win32::PTIMERAPCROUTINE callback,
			void* param
		) -> void
		{
			Win32::LARGE_INTEGER liDueTime;
			liDueTime.QuadPart = time;
			//https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-setwaitabletimer
			bool succeeded = Win32::SetWaitableTimer(
				m_handle.GetHandle(),
				&liDueTime,
				period,
				callback,
				param,
				false
			);
			if (not succeeded)
				throw Error::Win32Error{Win32::GetLastError(), "Failed to set timer"};
		}

		RAII::SharedHandle m_handle;
		std::wstring m_name;
		bool m_isManualReset = false;
	};
}