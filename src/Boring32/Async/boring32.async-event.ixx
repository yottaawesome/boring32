export module boring32.async:event;
import boring32.raii;
import <string>;
import <stdexcept>;
import <iostream>;
import <format>;
import <win32.hpp>;
import boring32.error;

export namespace Boring32::Async
{
	/// <summary>
	/// Encapsulates a Win32 Event synchronization object.
	/// </summary>
	class Event
	{
		// Constructors
		public:
			virtual ~Event() = default;
			Event() = default;
			Event(const Event& other) = default;
			Event(Event&& other) noexcept = default;

			/// <summary>
			///		Constructor for an anonymous Event object.
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
			Event(
				const bool isInheritable,
				const bool manualReset,
				const bool isSignaled
			) : m_isManualReset(manualReset)
			{
				InternalCreate(isSignaled, isInheritable);
			}

			/// <summary>
			///		Constructor for a named or anonymous Event object.
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
			///		If this parameter matches the name of an existing Event, the object attempts to
			///		open this existing Event with the EVENT_ALL_ACCESS privilege.
			/// </param>
			Event(
				const bool isInheritable,
				const bool manualReset,
				const bool isSignaled,
				std::wstring name
			) : m_isManualReset(manualReset),
				m_name(std::move(name))
			{
				InternalCreate(isSignaled, isInheritable);
			}

			Event(
				const bool isInheritable,
				const bool manualReset,
				std::wstring name,
				const DWORD desiredAccess
			) : m_isManualReset(manualReset),
				m_name(std::move(name))
			{
				m_event = OpenEventW(desiredAccess, isInheritable, m_name.c_str());
				if (!m_event)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("Failed to create or open event", lastError);
				}
			}

		public:
			virtual Event& operator=(const Event& other) = default;
			virtual Event& operator=(Event&& other) noexcept = default;
			virtual operator HANDLE() const noexcept
			{
				return *m_event;
			}

			virtual operator bool() const noexcept
			{
				return m_event != nullptr;
			}

			// API
		public:
			virtual void Signal()
			{
				if (!m_event)
					throw Error::Boring32Error("No Event to signal");
				if (!SetEvent(m_event.GetHandle()))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("Failed to signal event", lastError);
				}
			}

			virtual bool Signal(const std::nothrow_t&) noexcept try
			{
				Signal();
				return true;
			}
			catch (const std::exception& ex)
			{
				std::wcerr << std::format(
					"{}: Signal() failed: {}",
					std::source_location::current().function_name(),
					ex.what()
				).c_str();
				return false;
			}

			virtual void Reset()
			{
				if (!m_isManualReset)
					return;
				if (!m_event)
					return;
				if (!ResetEvent(m_event.GetHandle()))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("ResetEvent() failed", lastError);
				}
			}

			virtual bool Reset(std::nothrow_t) noexcept try
			{
				Reset();
				return true;
			}
			catch (const std::exception& ex)
			{
				std::wcerr << std::format(
					"{}: Reset() failed: {}",
					std::source_location::current().function_name(),
					ex.what()
				).c_str();
				return false;
			}

			virtual void WaitOnEvent() const
			{
				if (!m_event)
					throw Error::Boring32Error("No Event to wait on");

				const DWORD status = WaitForSingleObject(m_event.GetHandle(), INFINITE);
				if (status == WAIT_FAILED)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("WaitForSingleObject failed", lastError);
				}
				if (status == WAIT_ABANDONED)
					throw Error::Boring32Error("The wait was abandoned");
			}

			virtual bool WaitOnEvent(
				const DWORD millis, 
				const bool alertable
			) const
			{
				if (!m_event)
					throw Error::Boring32Error("No Event to wait on");

				const DWORD status = WaitForSingleObjectEx(m_event.GetHandle(), millis, alertable);
				if (status == WAIT_OBJECT_0)
					return true;
				if (status == WAIT_TIMEOUT)
					return false;
				if (status == WAIT_FAILED)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("WaitForSingleObject() failed", lastError);
				}
				if (status == WAIT_ABANDONED)
					throw Error::Boring32Error("The wait was abandoned");
				return false;
			}

			virtual bool WaitOnEvent(
				const DWORD millis, 
				const bool alertable, 
				const std::nothrow_t&
			) const noexcept try
			{
				//https://codeyarns.com/tech/2018-08-22-how-to-get-function-name-in-c.html
				WaitOnEvent();
				return true;
			}
			catch (const std::exception& ex)
			{
				std::wcerr << std::format(
					"{}: WaitOnEvent() failed: {}",
					std::source_location::current().function_name(),
					ex.what()
				).c_str();
				return false;
			}

			virtual HANDLE Detach() noexcept
			{
				return m_event.Detach();
			}

			virtual HANDLE GetHandle() const noexcept
			{
				return m_event.GetHandle();
			}

			virtual void Close()
			{
				m_event = nullptr;
			}

			virtual const std::wstring& GetName() const noexcept
			{
				return m_name;
			}

			virtual bool IsManualReset() const noexcept
			{
				return m_isManualReset;
			}

		protected:
			virtual void InternalCreate(
				const bool isSignaled, 
				const bool isInheritable
			)
			{
				m_event = CreateEventW(
					nullptr,			// security attributes
					m_isManualReset,	// manual reset event
					isSignaled,			// is initially signalled
					m_name.empty() ? nullptr : m_name.c_str() // name
				);
				if (!m_event)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("Failed to create or open event", lastError);
				}
				m_event.SetInheritability(isInheritable);
			}

		protected:
			RAII::Win32Handle m_event;
			bool m_isManualReset = false;
			std::wstring m_name;
	};
}