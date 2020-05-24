#pragma once
#include <Windows.h>
#include <string>
#include "../Raii/Raii.hpp"

namespace Boring32::Async
{
	class WaitableTimer
	{
		public:
			virtual ~WaitableTimer();

			WaitableTimer();
			WaitableTimer(const bool createNew, const std::wstring& name, const bool isInheritable, const bool isManualReset);

			WaitableTimer(const WaitableTimer& other);
			virtual void operator=(const WaitableTimer& other);
			virtual void Copy(const WaitableTimer& other);

			WaitableTimer(WaitableTimer&& other) noexcept;
			virtual void operator=(WaitableTimer&& other) noexcept;
			virtual void Move(WaitableTimer& other) noexcept;

			virtual void Close();

			virtual void SetTimerInNanos(const int64_t hundredNanosecondIntervals);
			virtual void SetTimerInMillis(const int64_t milliseconds);
			virtual bool WaitOnTimer(const DWORD millis);
			virtual bool CancelTimer();

		protected:
			virtual void InternalSetTimer(const int64_t time);

		protected:
			Raii::Win32Handle m_handle;
			std::wstring m_name;
			bool m_isManualReset;
	};
}