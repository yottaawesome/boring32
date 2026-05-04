export module boring32:async.event;
import std;
import :win32;
import :raii;
import :error;
import :concepts;

export namespace Boring32::Async
{
	/// Encapsulates a Win32 Event synchronization object.
	template<bool VIsManualReset = false>
	class Event final
	{
	public:
		Event() = default;
		Event(const Event& other) = delete;
		auto operator=(const Event& other) -> Event& = delete;
		Event(Event&& other) noexcept = default;
		auto operator=(Event&& other) noexcept -> Event& = default;

		// Custom constructors
		//	Constructor for an anonymous Event object.
		Event(bool isInheritable, bool isSignaled)
		{
			InternalCreate(isSignaled, isInheritable);
		}

		//	Constructor for a named or anonymous Event object.
		Event(bool isInheritable, bool isSignaled, std::wstring name) 
			: m_name(std::move(name))
		{
			InternalCreate(isSignaled, isInheritable);
		}

		Event(bool isInheritable, std::wstring name, Win32::DWORD desiredAccess) 
			: m_name(std::move(name))
		{
			if (m_name->empty())
				throw Error::Boring32Error("Name of Event to open cannot be empty");

			m_event = Win32::OpenEventW(desiredAccess, isInheritable, m_name->c_str());
			if (not m_event)
				throw Error::Win32Error{ Win32::GetLastError(), "Failed to create or open event" };
		}

		constexpr operator Win32::HANDLE() const noexcept
		{
			return *m_event;
		}

		constexpr operator bool() const noexcept
		{
			return m_event != nullptr;
		}

		void Signal()
		{
			if (not m_event)
				throw Error::Boring32Error("No Event to signal");
			if (not Win32::SetEvent(m_event.GetHandle()))
				throw Error::Win32Error{ Win32::GetLastError(), "Failed to signal event" };
		}

		auto Signal(const std::nothrow_t&) noexcept -> std::expected<void, std::string>
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
				throw Error::Win32Error{ Win32::GetLastError(), "ResetEvent() failed" };
		}

		auto Reset(const std::nothrow_t&) noexcept -> std::expected<void, std::string> 
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

			auto status = static_cast<Win32::WaitResult>(Win32::WaitForSingleObject(m_event.GetHandle(), Win32::Infinite));
			if (status == Win32::WaitResult::Failed)
				throw Error::Win32Error{Win32::GetLastError(), "WaitForSingleObject failed"};
			if (status == Win32::WaitResult::Abandoned)
				throw Error::Boring32Error{"The wait was abandoned"};
		}

		auto WaitOnEvent(Win32::DWORD millis, bool interruptible) const -> bool
		{
			return WaitOnEvent(std::chrono::milliseconds(millis), interruptible);
		}

		auto WaitOnEvent(Concepts::Duration auto&& time, bool alertable) const -> bool
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
					throw Error::Win32Error{ Win32::GetLastError(), "WaitForSingleObject() failed" };
			}
			return false;
		}

		auto WaitOnEvent(
			Concepts::Duration auto&& time, 
			bool alertable, 
			const std::nothrow_t&
		) const noexcept -> bool
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

		constexpr auto Detach() noexcept -> Win32::HANDLE
		{
			return m_event.Detach();
		}

		constexpr auto GetHandle() const noexcept -> Win32::HANDLE
		{
			return m_event.GetHandle();
		}

		constexpr void Close()
		{
			m_event = nullptr;
		}

		constexpr auto GetName() const noexcept -> std::optional<std::wstring>
		{
			return m_name.has_value() ? m_name : std::nullopt;
		}

		constexpr auto IsManualReset() const noexcept -> bool
		{
			return VIsManualReset;
		}

		auto IsSignalled() const -> bool
		{
			return WaitOnEvent(std::chrono::milliseconds{ 0 }, false);
		}

	private:
		void InternalCreate(bool isSignaled, bool isInheritable)
		{
			m_event = Win32::CreateEventW(
				nullptr,			// security attributes
				VIsManualReset,	// manual reset event
				isSignaled,			// is initially signalled
				m_name and not m_name->empty() ? m_name->c_str() : nullptr // name
			);
			if (not m_event)
				throw Error::Win32Error{ Win32::GetLastError(), "Failed to create or open event" };
			m_event.SetInheritability(isInheritable);
		}

		RAII::UniqueHandle m_event;
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