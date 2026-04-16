export module boring32:async.mutex;
import std;
import :win32;
import :error;
import :raii;
import :concepts;
import :async.functions;

export namespace Boring32::Async
{
	///	Represents a Win32 mutex, an object used for interprocess
	///	synchronisation and communication.
	class Mutex final
	{
	public:
		~Mutex()
		{
			Close();
		}

		/// Default constructible and moveable.
		Mutex() = default;
		Mutex(Mutex&& other) noexcept = default;
		auto operator=(Mutex&& other) noexcept -> Mutex& = default;

		///	Creates an anonymous mutex.
		Mutex(bool acquire, bool inheritable)
		{
			m_mutex = Win32::CreateMutexW(nullptr, acquire, nullptr);
			m_mutex.SetInheritability(inheritable);
			if (not m_mutex)
				throw Error::Win32Error{Win32::GetLastError(), "Failed to create mutex"};
		}

		///	Creates a new named mutex.
		Mutex(bool acquireOnCreation, bool inheritable, std::wstring name)
			: m_name(std::move(name))
		{
			if (m_name->empty())
				throw Error::Boring32Error{ "Cannot create mutex with empty name" };
			m_mutex = Win32::CreateMutexW(nullptr, acquireOnCreation, m_name->c_str());
			m_mutex.SetInheritability(inheritable);
			if (not m_mutex)
				throw Error::Win32Error{Win32::GetLastError(), "Failed to create mutex"};
		}

		///	Opens an existing named mutex.
		Mutex(bool acquireOnOpen, bool isInheritable, std::wstring name, Win32::DWORD desiredAccess) 
			: m_name(name)
		{
			if (m_name->empty())
				throw Error::Boring32Error("Cannot open mutex with empty name");
			m_mutex = Win32::OpenMutexW(desiredAccess, isInheritable, m_name->c_str());
			if (not m_mutex)
				throw Error::Win32Error{Win32::GetLastError(), "Failed to open mutex"};
			if (acquireOnOpen)
				Lock(Win32::Infinite, true);
		}

		auto Lock() -> bool
		{
			return Lock(Win32::Infinite, false);
		}

		auto Lock(Win32::DWORD waitTime) -> bool
		{
			return Lock(waitTime, false);
		}

		auto Lock(bool isAlertable) -> bool
		{
			return Lock(Win32::Infinite, isAlertable);
		}

		auto Lock(Concepts::Duration auto time, bool alertable) -> bool
		{
			using namespace std::chrono;
			return Lock(static_cast<Win32::DWORD>(duration_cast<milliseconds>(time).count()), alertable);
		}

		///	Blocks the current thread for a specified amount of time 
		///	(or indefinitely) until the mutex is acquired.
		auto Lock(Win32::DWORD waitTime, bool isAlertable) -> bool
		{
			if (not m_mutex)
				throw Error::Boring32Error("Cannot wait on null mutex");
			return WaitFor(m_mutex.GetHandle(), waitTime, isAlertable) == Win32::WaitResult::Success;
		}

		///	Blocks the current thread for a specified amount of time 
		///	(or indefinitely) until the mutex is acquired. Does not
		///	throw exceptions on failure.
		auto Lock(Win32::DWORD waitTime, bool isAlertable, const std::nothrow_t&) noexcept -> bool
		try
		{
			return Lock(waitTime, isAlertable);
		}
		catch (const std::exception&)
		{
			return false;
		}

		///	Frees the mutex, allowing another process to acquire it.
		///	Throws if the calling thread does not own the mutex.
		void Unlock()
		{
			if (not m_mutex)
				throw Error::Boring32Error("Cannot release null mutex");
			if (not Win32::ReleaseMutex(m_mutex.GetHandle()))
				throw Error::Win32Error{Win32::GetLastError(), "Failed to release mutex"};
		}

		///	Attempts to release the mutex. Returns true if the mutex
		///	was successfully released, or false if the calling thread
		///	does not own it. Does not throw.
		auto TryUnlock() noexcept -> bool
		{
			if (not m_mutex)
				return false;
			return Win32::ReleaseMutex(m_mutex.GetHandle());
		}

		///	Frees the mutex, allowing another process to acquire it.
		///	Does not throw exceptions on failure.
		auto Unlock(const std::nothrow_t&) noexcept -> bool
		try
		{
			Unlock();
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}

		///	Invalidates and closes the native Mutex handle.
		void Close()
		{
			if (not m_mutex)
				return;
			// Best-effort release; ignore failure if not owned.
			Win32::ReleaseMutex(m_mutex.GetHandle());
			m_mutex.Close();
		}

		///	Retrieves this Mutex's underlying handle.
		[[nodiscard]]
		auto GetHandle() const noexcept -> Win32::HANDLE
		{
			return m_mutex.GetHandle();
		}

		///	Retrieves this Mutex's name. This value is an empty string
		///	if this is an anonymous Mutex.
		[[nodiscard]]
		auto GetName() const noexcept -> std::optional<std::wstring>
		{
			return m_name;
		}

	private:
		std::optional<std::wstring> m_name = std::nullopt;
		RAII::UniqueHandle m_mutex;
	};
}