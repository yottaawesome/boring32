export module boring32.async:event;
import <string>;
import <stdexcept>;
import <iostream>;
import <chrono>;
import <format>;
import <optional>;
import <source_location>;
import <win32.hpp>;
import boring32.raii;
import boring32.error;
import :concepts;

export namespace Boring32::Async
{
	/// <summary>
	/// Encapsulates a Win32 Event synchronization object.
	/// </summary>
	class Event final
	{
		// The Six
		public:
			~Event() = default;
			Event() = default;
			Event(const Event& other) = default;
			Event(Event&& other) noexcept = default;
			Event& operator=(const Event& other) = default;
			Event& operator=(Event&& other) noexcept = default;

		// Custom constructors
		public:
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
				if (m_name->empty())
					throw Error::Boring32Error("Name of Event to open cannot be empty");

				m_event = OpenEventW(
					desiredAccess, 
					isInheritable, 
					m_name->c_str()
				);
				if (!m_event)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("Failed to create or open event", lastError);
				}
			}

		public:
			inline operator HANDLE() const noexcept
			{
				return *m_event;
			}

			inline operator bool() const noexcept
			{
				return m_event != nullptr;
			}

			// API
		public:
			void Signal()
			{
				if (!m_event)
					throw Error::Boring32Error("No Event to signal");
				if (!SetEvent(m_event.GetHandle()))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("Failed to signal event", lastError);
				}
			}

			bool Signal(const std::nothrow_t&) noexcept try
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

			void Reset()
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

			bool Reset(const std::nothrow_t&) noexcept try
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

			void WaitOnEvent() const
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

			bool WaitOnEvent(const DWORD millis, const bool interruptible) const
			{
				return WaitOnEvent(std::chrono::milliseconds(millis), interruptible);
			}

			bool WaitOnEvent(
				const Duration auto& time,
				const bool alertable
			) const
			{
				using std::chrono::duration_cast;
				using std::chrono::milliseconds;

				if (!m_event)
					throw Error::Boring32Error("No Event to wait on");

				const unsigned long millis = static_cast<unsigned long>(duration_cast<milliseconds>(time).count());

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

			bool WaitOnEvent(
				const Duration auto& time,
				const bool alertable, 
				const std::nothrow_t&
			) const noexcept try
			{
				//https://codeyarns.com/tech/2018-08-22-how-to-get-function-name-in-c.html
				WaitOnEvent(time, alertable);
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

			inline HANDLE Detach() noexcept
			{
				return m_event.Detach();
			}

			inline HANDLE GetHandle() const noexcept
			{
				return m_event.GetHandle();
			}

			inline void Close()
			{
				m_event = nullptr;
			}

			inline const std::optional<std::wstring>& GetName() const noexcept
			{
				return m_name;
			}

			inline bool IsManualReset() const noexcept
			{
				return m_isManualReset;
			}

		private:
			void InternalCreate(
				const bool isSignaled, 
				const bool isInheritable
			)
			{
				m_event = CreateEventW(
					nullptr,			// security attributes
					m_isManualReset,	// manual reset event
					isSignaled,			// is initially signalled
					!m_name || m_name->empty() ? nullptr : m_name->c_str() // name
				);
				if (!m_event)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("Failed to create or open event", lastError);
				}
				m_event.SetInheritability(isInheritable);
			}

		private:
			RAII::Win32Handle m_event;
			bool m_isManualReset = false;
			std::optional<std::wstring> m_name;
	};
}