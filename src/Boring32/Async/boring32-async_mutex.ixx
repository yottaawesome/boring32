export module boring32:async_mutex;
import boring32.shared;
import :error;
import :raii;
import :concepts;
import :async_functions;

export namespace Boring32::Async
{
	///		Represents a Win32 mutex, an object used for interprocess
	///		synchronisation and communication.
	struct Mutex final
	{
		/// Default constructor. Does not initialise any underlying mutex.
		Mutex() = default;

		///		Move constructor.
		Mutex(Mutex&& other) noexcept
		{
			Move(other);
		}

		///		Move assignment.
		Mutex& operator=(Mutex&& other) noexcept
		{
			Move(other);
			return *this;
		}

		~Mutex()
		{
			Close();
		}

		///		Creates an anonymous mutex.
		Mutex(bool acquire, bool inheritable)
			: m_locked(acquire)
		{
			m_mutex = Win32::CreateMutexW(nullptr, m_locked, nullptr);
			m_mutex.SetInheritability(inheritable);
			if (not m_mutex)
				throw Error::Win32Error(Win32::GetLastError(), "Failed to create mutex");
		}

		///		Creates a new named or anonymous mutex.
		Mutex(bool acquireOnCreation, bool inheritable, std::wstring name) 
			: m_name(std::move(name)),
			m_created(true)
		{
			m_mutex = Win32::CreateMutexW(
				nullptr,
				acquireOnCreation,
				m_name.empty() ? nullptr : m_name.c_str()
			);
			m_mutex.SetInheritability(inheritable);
			if (not m_mutex)
				throw Error::Win32Error(Win32::GetLastError(), "Failed to create mutex");

			m_locked = acquireOnCreation;
		}

		///		Opens an existing named mutex.
		Mutex(bool acquireOnOpen, bool isInheritable, std::wstring name, Win32::DWORD desiredAccess) 
			: m_name(name)
		{
			if (m_name.empty())
				throw Error::Boring32Error("Cannot open mutex with empty name");
			m_mutex = Win32::OpenMutexW(desiredAccess, isInheritable, m_name.c_str());
			if (not m_mutex)
				throw Error::Win32Error(Win32::GetLastError(), "Failed to open mutex");
			if (acquireOnOpen)
				Lock(Win32::Infinite, true);
		}

		bool Lock()
		{
			return Lock(Win32::Infinite, false);
		}

		bool Lock(Win32::DWORD waitTime)
		{
			return Lock(waitTime, false);
		}

		bool Lock(bool isAlertable)
		{
			return Lock(Win32::Infinite, isAlertable);
		}

		bool Lock(Concepts::Duration auto time, bool alertable)
		{
			using namespace std::chrono;
			return Lock(static_cast<Win32::DWORD>(duration_cast<milliseconds>(time).count()), alertable);
		}

		///		Blocks the current thread for a specified amount of time 
		///		(or indefinitely) until the mutex is acquired.
		bool Lock(Win32::DWORD waitTime, bool isAlertable)
		{
			if (not m_mutex)
				throw Error::Boring32Error("Cannot wait on null mutex");
			return m_locked = WaitFor(m_mutex.GetHandle(), waitTime, isAlertable) == Win32::WaitResult::Success;
		}

		///		Blocks the current thread for a specified amount of time 
		///		(or indefinitely) until the mutex is acquired. Does not
		///		throw exceptions on failure.
		bool Lock(Win32::DWORD waitTime, bool isAlertable, std::nothrow_t) noexcept 
		try
		{
			return Lock(waitTime, isAlertable);
		}
		catch (const std::exception&)
		{
			return false;
		}

		///		Frees the mutex, allowing another process to acquire it.
		void Unlock()
		{
			if (not m_mutex)
				throw Error::Boring32Error("Cannot wait on null mutex");
			if (not Win32::ReleaseMutex(m_mutex.GetHandle()))
				throw Error::Win32Error(Win32::GetLastError(), "Failed to release mutex");

			m_locked = false;
		}

		///		Frees the mutex, allowing another process to acquire it.
		///		Does not throw exceptions on failure.
		bool Unlock(const std::nothrow_t&) noexcept 
		try
		{
			Unlock();
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}

		///		Invalidates and closes the native Mutex handle.
		void Close()
		{
			if (not m_mutex)
				return;
			if (m_locked)
				Unlock();
			m_mutex.Close();
		}

		///		Retrieves this Mutex's underlying handle.
		Win32::HANDLE GetHandle() const noexcept
		{
			return m_mutex.GetHandle();
		}

		///	Retrieves this Mutex's name. This value is an empty string
		///	if this is an anonymous Mutex.
		const std::wstring& GetName() const noexcept
		{
			return m_name;
		}

		private:
		void Move(Mutex& other) noexcept
		{
			Close();
			m_name = std::move(other.m_name);
			m_created = other.m_created;
			m_locked = other.m_locked.load(); // TODO: probably pointless, remove
			m_mutex = std::move(other.m_mutex);
		}

		std::wstring m_name;
		bool m_created = false;
		std::atomic<bool> m_locked = false;
		RAII::Win32Handle m_mutex;
	};
}