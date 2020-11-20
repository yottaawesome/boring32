#pragma once
#include <Windows.h>
#include <string>
#include "../Raii/Win32Handle.hpp"

namespace Boring32::Async
{
	/// <summary>
	/// Encapsulates a Win32 Event synchronization object.
	/// </summary>
	class Event
	{
		// Constructors
		public:
			virtual ~Event();
			Event();

			/// <summary>
			///		Constructor for an Event object.
			/// </summary>
			/// <param name="isInheritable">
			///		Whether this Event is inheritable by any child processes.
			/// </param>
			/// <param name="manualReset">
			///		Whether this is an auto-reset or manual reset Event.
			/// </param>
			/// <param name="isSignaled">
			///		Whether this Event is initially signalled on creation.
			/// </param>
			/// <param name="name">
			///		The name of this Event object. Pass an empty string to create an anonymous Event.
			/// </param>
			Event(
				const bool isInheritable, 
				const bool manualReset, 
				const bool isSignaled, 
				std::wstring name
			);
			
			Event(
				const bool isInheritable,
				const bool manualReset,
				std::wstring name,
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
			virtual bool WaitOnEvent(const DWORD millis, const bool alertable);
			virtual HANDLE Detach();
			virtual HANDLE GetHandle() const;
			virtual void Close();
			virtual const std::wstring& GetName() const;

		protected:
			virtual void Copy(const Event& other);
			virtual void Move(Event& other) noexcept;

		protected:
			Raii::Win32Handle m_event;
			bool m_isManualReset;
			bool m_createEventOnTrue;
			std::wstring m_name;
			DWORD m_access;
	};
}