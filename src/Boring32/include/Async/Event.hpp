#pragma once
#include <Windows.h>
#include <string>
#include "../Raii/Win32Handle.hpp"

namespace Boring32::Async
{
	class Event
	{
		// Constructors
		public:
			virtual ~Event();
			Event();
			Event(
				const bool createOrOpen,
				const bool isInheritable, 
				const bool manualReset, 
				const bool isSignaled, 
				const std::wstring& name
			);

			Event(const Event& other);
			virtual void operator=(const Event& other);

			Event(Event&& other) noexcept;
			virtual void operator=(Event&& other) noexcept;

		// API
		public:
			virtual void Close();
			virtual void Reset();
			virtual HANDLE GetHandle();

			virtual void WaitOnEvent();
			virtual bool WaitOnEvent(const DWORD millis);

			virtual bool Signal();

		protected:
			virtual void Duplicate(const Event& other);
			virtual void Move(Event& other) noexcept;

		protected:
			Raii::Win32Handle m_event;
			bool m_isManualReset;
			bool m_isSignaled;
			bool m_createEventOnTrue;
			std::wstring m_name;
	};
}