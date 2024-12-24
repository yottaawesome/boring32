export module boring32:async_event;
import boring32.shared;
import :raii;
import :error;
import :concepts;

export namespace Boring32::Async
{
	/// Encapsulates a Win32 Event synchronization object.
	template<bool VIsManualReset = false>
	struct Event final
	{
		Event() = default;
		Event(const Event& other) = default;
		Event(Event&& other) noexcept = default;
		Event& operator=(const Event& other) = default;
		Event& operator=(Event&& other) noexcept = default;

		// Custom constructors
		//	Constructor for an anonymous Event object.
		Event(const bool isInheritable, const bool isSignaled)
		{
			InternalCreate(isSignaled, isInheritable);
		}

		//	Constructor for a named or anonymous Event object.
		Event(const bool isInheritable, const bool isSignaled, std::wstring name) 
			: m_name(std::move(name))
		{
			InternalCreate(isSignaled, isInheritable);
		}

		Event(const bool isInheritable, std::wstring name, const Win32::DWORD desiredAccess) 
			: m_name(std::move(name))
		{
			if (m_name->empty())
				throw Error::Boring32Error("Name of Event to open cannot be empty");

			m_event = Win32::OpenEventW(desiredAccess, isInheritable, m_name->c_str());
			if (not m_event)
				throw Error::Win32Error(Win32::GetLastError(), "Failed to create or open event");
		}

		operator Win32::HANDLE() const noexcept
		{
			return *m_event;
		}

		operator bool() const noexcept
		{
			return m_event != nullptr;
		}

		void Signal()
		{
			if (not m_event)
				throw Error::Boring32Error("No Event to signal");
			if (not Win32::SetEvent(m_event.GetHandle()))
				throw Error::Win32Error(Win32::GetLastError(), "Failed to signal event");
		}

		std::expected<void, std::string> Signal(const std::nothrow_t&) noexcept 
		try
		{
			Signal();
			return {};
		}
		catch (const std::exception& ex)
		{
			return std::unexpected(ex.what());
		}

		void Reset() requires VIsManualReset
		{
			if (not m_event)
				return;
			if (not Win32::ResetEvent(m_event.GetHandle()))
				throw Error::Win32Error(Win32::GetLastError(), "ResetEvent() failed");
		}

		std::expected<void, std::string> Reset(const std::nothrow_t&) noexcept 
			requires VIsManualReset
		try
		{
			Reset();
			return {};
		}
		catch (const std::exception& ex)
		{
			return std::unexpected(ex.what());
		}

		void WaitOnEvent() const
		{
			if (not m_event)
				throw Error::Boring32Error("No Event to wait on");

			Win32::WaitResult status = static_cast<Win32::WaitResult>(Win32::WaitForSingleObject(m_event.GetHandle(), Win32::Infinite));
			if (status == Win32::WaitResult::Failed)
				throw Error::Win32Error(Win32::GetLastError(), "WaitForSingleObject failed");
			if (status == Win32::WaitResult::Abandoned)
				throw Error::Boring32Error("The wait was abandoned");
		}

		bool WaitOnEvent(const Win32::DWORD millis, const bool interruptible) const
		{
			return WaitOnEvent(std::chrono::milliseconds(millis), interruptible);
		}

		bool WaitOnEvent(const Concepts::Duration auto& time, const bool alertable) const
		{
			using std::chrono::duration_cast;
			using std::chrono::milliseconds;

			if (not m_event)
				throw Error::Boring32Error("No Event to wait on");

			unsigned long millis = static_cast<unsigned long>(duration_cast<milliseconds>(time).count());
			switch (Win32::WaitResult status = static_cast<Win32::WaitResult>(Win32::WaitForSingleObjectEx(m_event.GetHandle(), millis, alertable)))
			{
				case Win32::WaitResult::Success: 
					return true;
				case Win32::WaitResult::Timeout: 
					return false;
				case Win32::WaitResult::Abandoned:
					throw Error::Boring32Error("The wait was abandoned");
				case Win32::WaitResult::Failed: 
					throw Error::Win32Error(Win32::GetLastError(), "WaitForSingleObject() failed");
			}
			return false;
		}

		bool WaitOnEvent(const Concepts::Duration auto& time, const bool alertable, const std::nothrow_t&) const noexcept 
		try
		{
			//https://codeyarns.com/tech/2018-08-22-how-to-get-function-name-in-c.html
			WaitOnEvent(time, alertable);
			return true;
		}
		catch (const std::exception& ex)
		{
			return false;
		}

		Win32::HANDLE Detach() noexcept
		{
			return m_event.Detach();
		}

		Win32::HANDLE GetHandle() const noexcept
		{
			return m_event.GetHandle();
		}

		void Close()
		{
			m_event = nullptr;
		}

		const std::optional<std::wstring>& GetName() const noexcept
		{
			return m_name;
		}

		constexpr bool IsManualReset() const noexcept
		{
			return VIsManualReset;
		}

		private:
		void InternalCreate(const bool isSignaled, const bool isInheritable)
		{
			m_event = Win32::CreateEventW(
				nullptr,			// security attributes
				VIsManualReset,	// manual reset event
				isSignaled,			// is initially signalled
				m_name and not m_name->empty() ? m_name->c_str() : nullptr // name
			);
			if (not m_event)
				throw Error::Win32Error(Win32::GetLastError(), "Failed to create or open event");
			m_event.SetInheritability(isInheritable);
		}

		RAII::Win32Handle m_event;
		std::optional<std::wstring> m_name;
	};

	template<typename T> 
	struct IsAnyEventT : std::false_type {};
	template<bool VIsManualReset> 
	struct IsAnyEventT<Event<VIsManualReset>> : std::true_type {};
	template<typename T>
	constexpr bool IsAnyEventV = IsAnyEventT<T>::value;
	template<typename T>
	concept AnyEvent = IsAnyEventV<T>;

	using ManualResetEvent = Event<true>;
	using AutoResetEvent = Event<false>;
}