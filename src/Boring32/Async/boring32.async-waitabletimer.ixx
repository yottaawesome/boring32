module;

#include <string>
#include <Windows.h>

export module boring32.async:waitabletimer;
import boring32.raii;

export namespace Boring32::Async
{
	class WaitableTimer
	{
		public:
			virtual ~WaitableTimer();

			WaitableTimer();
			WaitableTimer(
				std::wstring name, 
				const bool isInheritable, 
				const bool isManualReset
			);
			WaitableTimer(
				std::wstring name, 
				const bool isInheritable, 
				const bool isManualReset, 
				const DWORD desiredAccess
			);

			WaitableTimer(const WaitableTimer& other);
			virtual void operator=(const WaitableTimer& other);
			virtual void Copy(const WaitableTimer& other);

			WaitableTimer(WaitableTimer&& other) noexcept;
			virtual void operator=(WaitableTimer&& other) noexcept;
			virtual void Move(WaitableTimer& other) noexcept;

		public:
			/// <summary>
			///		Set this timer using 100-nanosecond intervals.
			/// </summary>
			/// <param name="hundredNanosecondIntervals">
			///		The time after which the state of the timer is to be set to signaled, in 100 nanosecond intervals.
			///		Positive values indicate absolute time. Be sure to use a UTC-based absolute time, 
			///		as the system uses UTC-based time internally. Negative values indicate relative time. 
			///		The actual timer accuracy depends on the capability of your hardware.
			/// </param>
			/// <param name="period">
			///		The period of the timer, in milliseconds. If period is zero,
			///		the timer is signaled once, else if it is greater than zero, the
			///		timer is periodic. A periodic timer automatically reactivates
			///		each time the period elapses, until the timer is canceled.
			///	</param>
			virtual void SetTimerInNanos(
				const int64_t hundredNanosecondIntervals, 
				const UINT period,
				const PTIMERAPCROUTINE callback,
				void* param
			);

			virtual bool SetTimerInNanos(
				const int64_t hundredNanosecondIntervals,
				const UINT period,
				const PTIMERAPCROUTINE callback,
				void* param,
				std::nothrow_t
			) noexcept;

			virtual void SetTimerInMillis(
				const int64_t milliseconds, 
				const UINT period,
				const PTIMERAPCROUTINE callback,
				void* param
			);

			virtual bool SetTimerInMillis(
				const int64_t milliseconds,
				const UINT period,
				const PTIMERAPCROUTINE callback,
				void* param,
				std::nothrow_t
			) noexcept;

			virtual bool WaitOnTimer(const DWORD millis);

			virtual bool WaitOnTimer(const DWORD millis, std::nothrow_t) noexcept;

			virtual void CancelTimer();

			virtual bool CancelTimer(std::nothrow_t) noexcept;

			virtual bool IsManualReset() const noexcept;

			virtual const std::wstring& GetName() const  noexcept;

			/// <summary>
			///		Returns the underlying HANDLE for this timer, or null
			///		if no timer has been created.
			/// </summary>
			/// <returns></returns>
			virtual HANDLE GetHandle() const noexcept;

			/// <summary>
			///		Frees all native resources associated with this timer.
			/// </summary>
			virtual void Close();

		protected:
			virtual void InternalSetTimer(
				const int64_t time, 
				const UINT period,
				const PTIMERAPCROUTINE callback, 
				void* param
			);

		protected:
			Raii::Win32Handle m_handle;
			std::wstring m_name;
			bool m_isManualReset;
	};
}