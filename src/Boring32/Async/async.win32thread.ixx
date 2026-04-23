export module boring32:async.win32thread;
import std;
import :win32;
import :error;
import :raii;

export namespace Boring32::Async
{
	// TODO: Convert to deducing this. For now, subclasses should override Run().
	// Do not call Start() from base class constructor, only derived constructors should call it.
	// Derived classes must Close() or Join() in their own destructor for proper cleanup, as by 
	// the time the base class destructor is called, the derived class's members will have been 
	// destroyed and the thread procedure may be accessing destroyed members if it hasn't finished 
	// yet.
	class Win32Thread
	{
	public:
		virtual ~Win32Thread()
		try
		{
			Close();
		}
		catch (...)
		{
			std::cerr << "Exception while Closing() thread in destructor." << std::endl;
			std::terminate();
		}

		Win32Thread() = default;

		Win32Thread(const Win32Thread&) = delete;
		auto operator=(const Win32Thread&) -> Win32Thread& = delete;

		Win32Thread(Win32Thread&& other) noexcept = default;
		auto operator=(Win32Thread&& other) noexcept -> Win32Thread&
		{
			Close();
			m_threadHandle = std::move(other.m_threadHandle);
			return *this;
		}

		///	Terminates the thread. Be careful when using this
		///	function, as it prevents proper clean up of the 
		///	thread's objects and may leave shared objects in  
		///	an inconsistent state. Note also that if a thread
		///	is waiting on a kernel object, it will not be 
		///	terminated until the wait is finished.
		void Terminate(Win32::DWORD exitCode)
		{
			if (not m_threadHandle)
				throw Error::Boring32Error("No thread handle to terminate");
			if (not Win32::TerminateThread(m_threadHandle.GetHandle(), exitCode))
				throw Error::Win32Error{Win32::GetLastError(), "TerminateThread() failed"};
		}

		auto Suspend() -> Win32::DWORD
		{
			constexpr auto Failure = static_cast<Win32::DWORD>(-1);
			if (not m_threadHandle)
				throw Error::Boring32Error("No thread handle to suspend");
			auto count = Win32::SuspendThread(m_threadHandle.GetHandle());
			if (count == Failure)
				throw Error::Win32Error{Win32::GetLastError(), "SuspendThread() failed"};
			return count;
		}

		auto Resume() -> Win32::DWORD
		{
			constexpr auto Failure = static_cast<Win32::DWORD>(-1);
			if (not m_threadHandle)
				throw Error::Boring32Error("No thread handle to resume");
			auto count = Win32::ResumeThread(m_threadHandle.GetHandle());
			if (count == Failure)
				throw Error::Win32Error{Win32::GetLastError(), "ResumeThread() failed"};
			return count;
		}

		auto Join(Win32::DWORD joinTime) -> bool
		{
			if (m_threadHandle == nullptr)
				throw Error::Boring32Error("No thread handle to wait on");

			auto waitResult = Win32::WaitForSingleObject(m_threadHandle.GetHandle(), joinTime);
			if (waitResult == Win32::WaitObject0)
				return true;
			if (waitResult == Win32::WaitTimeout)
				return false;
			// Win32::WaitAbandoned is only returned when waiting on a mutex, so shouldn't be possible here
			throw Error::Win32Error{Win32::GetLastError(), "WaitForSingleObject() failed"};
		}

		enum class CloseResult
		{
			Success,
			Timeout
		};
		// Join the thread and close the handle if successful. If the wait times out, 
		// the handle is not closed and the caller can decide what to do about it. On
		// failure, the handle is not closed and the caller can inspect the error.
		auto TryClose(Win32::DWORD joinTime) noexcept -> std::expected<CloseResult, Win32::DWORD>
		{
			if (m_threadHandle == nullptr)
				return CloseResult::Success;

			auto waitResult = Win32::WaitForSingleObject(m_threadHandle.GetHandle(), joinTime);
			if (waitResult == Win32::WaitObject0)
			{
				m_threadHandle.Close();
				return CloseResult::Success;
			}
			if (waitResult == Win32::WaitTimeout)
				return CloseResult::Timeout;
			return std::unexpected{ Win32::GetLastError() };
		}

		void Close(Win32::DWORD joinTime = Win32::Infinite)
		{
			auto result = TryClose(joinTime);
			if (not result)
				throw Error::Win32Error{ result.error(), "Error joining thread during Close()." };
			if (*result == CloseResult::Timeout)
				throw Error::Boring32Error{ std::format("Timed out joining thread within {} milliseconds during Close().", joinTime) };
		}

		void Start()
		{
			if (m_threadHandle) 
				throw Error::Boring32Error{ "Thread already started" };
			// https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/beginthread-beginthreadex?view=vs-2019
			m_threadHandle = reinterpret_cast<Win32::HANDLE>(
				Win32::_beginthreadex(0, 0, Win32Thread::ThreadProc, this, 0, nullptr));
			if (not m_threadHandle)
			{
				auto errorCode = int{};
				Win32::_get_errno(&errorCode);
				throw Error::Boring32Error{ std::format("_beginthreadex() failed with error code {}", errorCode) };
			}
		}

		[[nodiscard]]
		auto IsRunning() const noexcept -> bool
		{
			if (not m_threadHandle)
				return false;
			return Win32::WaitForSingleObject(m_threadHandle.GetHandle(), 0) == Win32::WaitTimeout;
		}

		// Note that this will return STILL_ACTIVE (259) if the thread is still running.
		[[nodiscard]]
		auto GetExitCode() const -> Win32::DWORD
		{
			if (not m_threadHandle)
				throw Error::Boring32Error("No handle to thread; has the the thread been started or destroyed?");

			auto exitCode = Win32::DWORD{};
			if (not Win32::GetExitCodeThread(m_threadHandle.GetHandle(), &exitCode))
				throw Error::Win32Error{Win32::GetLastError(), "GetExitCodeThread() failed"};
			return exitCode;
		}

		[[nodiscard]]
		auto GetHandle() const noexcept -> Win32::HANDLE
		{
			return m_threadHandle.GetHandle();
		}

		// Relinquishes ownership of the thread handle without terminating the thread or closing the underlying native handle. 
		// The caller is responsible for closing the handle when it's no longer needed.
		[[nodiscard]]
		auto RelinquishHandle() noexcept -> Win32::HANDLE
		{
			return m_threadHandle.Detach();
		}

		// Closes the thread handle without terminating the thread, equivalent to std::thread::detach().
		void Detach() noexcept
		{
			m_threadHandle = nullptr;
		}

		void SetDescription(const std::wstring& description)
		{
			if (not m_threadHandle)
				throw Error::Boring32Error("No thread created.");

			auto hr = Win32::HRESULT{Win32::SetThreadDescription(m_threadHandle.GetHandle(), description.c_str())};
			if (Win32::HrFailed(hr))
				throw Error::COMError(hr, "SetThreadDescription() failed");
		}

		[[nodiscard]]
		auto GetDescription() const -> std::wstring
		{
			if (not m_threadHandle)
				throw Error::Boring32Error("No thread created.");

			wchar_t* pThreadDescription = {};
			// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getthreaddescription
			auto hr = Win32::HRESULT{Win32::GetThreadDescription(m_threadHandle.GetHandle(), &pThreadDescription)};
			if (Win32::HrFailed(hr))
				throw Error::COMError(hr, "GetThreadDescription() failed");
			if (not pThreadDescription)
				return {};
			auto deleter = RAII::LocalHeapUniquePtr<wchar_t>(pThreadDescription);
			return pThreadDescription;
		}

	protected:
		virtual auto Run() -> unsigned = 0;

		static auto ThreadProc(void* param) noexcept -> unsigned
		try
		{
			auto threadObj = static_cast<Win32Thread*>(param);
			auto returnCode = unsigned{};
			returnCode = threadObj->Run();

			return returnCode;
		}
		catch (...)
		{
			std::cerr << "Exception escaped from thread procedure. Terminating process." << std::endl;
			std::terminate();
		}

		RAII::UniqueHandle m_threadHandle;
	};
}