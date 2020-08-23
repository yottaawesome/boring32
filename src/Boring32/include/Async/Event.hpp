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
				const bool isInheritable, 
				const bool manualReset, 
				const bool isSignaled, 
				const std::wstring& name
			);
			Event(
				const bool isInheritable,
				const bool manualReset,
				const std::wstring& name,
				const DWORD desiredAccess
			);

			Event(const Event& other);
			virtual Event& operator=(const Event& other);

			Event(Event&& other) noexcept;
			virtual Event& operator=(Event&& other) noexcept;

		// API
		public:
			virtual void Signal();
			virtual void Reset();
			virtual void WaitOnEvent();
			virtual bool WaitOnEvent(const DWORD millis);
			virtual HANDLE Detach();
			virtual HANDLE GetHandle();
			virtual void Close();

		protected:
			virtual void Duplicate(const Event& other);
			virtual void Move(Event& other) noexcept;

		protected:
			Raii::Win32Handle m_event;
			bool m_isManualReset;
			bool m_createEventOnTrue;
			std::wstring m_name;
			DWORD m_access;
	};
}