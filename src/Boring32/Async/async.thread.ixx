export module boring32:async.thread;
import std;
import :win32;
import :error;
import :raii;
import :async.event;

export namespace Boring32::Async
{
	enum class ThreadStatus
	{
		Finished = 0,
		Ready = 1,
		Running = 2,
		Suspended = 3,
		Terminated = 4
	};

	// TODO: selectively devirtualise
	struct Thread
	{
		// Default constructible, movable, not copyable
		virtual ~Thread()
		{
			Close();
		}

		Thread() = default;

		Thread(void* param)
			: Thread()
		{
			m_threadParam = param;
		}

		Thread(Thread&& other) noexcept
		{
			Move(other);
		}

		Thread(const Thread&) = delete;

		virtual auto operator=(const Thread&) -> Thread& = delete;

		virtual auto operator=(Thread&& other) noexcept -> Thread&
		{
			Move(other);
			return *this;
		}

		virtual auto operator==(const ThreadStatus status) const noexcept -> bool
		{
			return m_status == status;
		}

		///	Terminates the thread. Be careful when using this
		///	function, as it prevents proper clean up of the 
		///	thread's objects and may leave shared objects in  
		///	an inconsistent state. Note also that if a thread
		///	is waiting on a kernel object, it will not be 
		///	terminated until the wait is finished.
		virtual auto Terminate(const Win32::DWORD exitCode) -> void
		{
			if (not m_threadHandle)
				throw Error::Boring32Error("No thread handle to terminate");
			if (not Win32::TerminateThread(m_threadHandle.GetHandle(), exitCode))
				throw Error::Win32Error{Win32::GetLastError(), "TerminateThread() failed"};
			m_status = ThreadStatus::Terminated;
		}

		virtual auto Suspend() -> void
		{
			if (not m_threadHandle)
				throw Error::Boring32Error("No thread handle to suspend");
			if (m_status != ThreadStatus::Running)
				throw Error::Boring32Error("Thread was not running when request to suspend occurred.");

			if (not Win32::SuspendThread(m_threadHandle.GetHandle()))
				throw Error::Win32Error{Win32::GetLastError(), "SuspendThread() failed"};
			m_status = ThreadStatus::Suspended;
		}

		virtual auto Resume() -> void
		{
			if (not m_threadHandle)
				throw Error::Boring32Error("No thread handle to resume");
			if (m_status != ThreadStatus::Suspended)
				throw Error::Boring32Error("Thread was not suspended when request to resume occurred.");
			if (not Win32::ResumeThread(m_threadHandle.GetHandle()))
				throw Error::Win32Error{Win32::GetLastError(), "ResumeThread() failed"};
			m_status = ThreadStatus::Running;
		}

		virtual auto Join(const Win32::DWORD waitTime) -> bool
		{
			if (m_threadHandle == nullptr)
				throw Error::Boring32Error("No thread handle to wait on");

			Win32::DWORD waitResult = Win32::WaitForSingleObject(m_threadHandle.GetHandle(), waitTime);
			if (waitResult == Win32::WaitObject0)
				return true;
			if (waitResult == Win32::WaitTimeout)
				return false;
			if (waitResult == Win32::WaitAbandoned)
				throw Error::Boring32Error("Wait was abandoned");
			throw Error::Win32Error{Win32::GetLastError(), "WaitForSingleObject() failed"};
		}

		virtual auto Close() -> void
		{
			m_threadHandle = nullptr;
		}
			
		virtual auto Start() -> void
		{
			InternalStart();
		}

		virtual auto Start(int(*simpleFunc)(void*)) -> void
		{
			m_func = simpleFunc;
			InternalStart();
		}

		virtual auto Start(const std::function<int(void*)>& func) -> void
		{
			m_func = func;
			InternalStart();
		}

		virtual auto GetStatus() const noexcept -> ThreadStatus
		{
			return m_status;
		}

		virtual auto GetExitCode() const -> unsigned long
		{
			if (not m_threadHandle)
				throw Error::Boring32Error("No handle to thread; has the the thread been started or destroyed?");

			Win32::DWORD exitCode;
			if (not Win32::GetExitCodeThread(m_threadHandle.GetHandle(), &exitCode))
				throw Error::Win32Error{Win32::GetLastError(), "GetExitCodeThread() failed"};
			return exitCode;
		}

		virtual auto GetHandle() const noexcept -> Win32::HANDLE
		{
			return m_threadHandle.GetHandle();
		}

		virtual auto WaitToStart(const Win32::DWORD millis) -> bool
		{
			return m_started.WaitOnEvent(millis, true);
		}

		virtual auto SetDescription(const std::wstring& description) -> void
		{
			if (not m_threadHandle)
				throw Error::Boring32Error("No thread created.");

			Win32::HRESULT hr = Win32::SetThreadDescription(
				m_threadHandle.GetHandle(),
				description.c_str()
			);
			if (Win32::HrFailed(hr))
				throw Error::COMError(hr, "SetThreadDescription() failed");
		}

		virtual auto GetDescription() -> std::wstring
		{
			if (not m_threadHandle)
				throw Error::Boring32Error("No thread created.");

			wchar_t* pThreadDescription = nullptr;
			// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getthreaddescription
			Win32::HRESULT hr = Win32::GetThreadDescription(m_threadHandle, &pThreadDescription);
			if (Win32::HrFailed(hr))
				throw Error::COMError(hr, "GetThreadDescription() failed");
			if (not pThreadDescription)
				return {};
			RAII::LocalHeapUniquePtr<wchar_t> deleter(pThreadDescription);
			return pThreadDescription;
		}

	protected:
		virtual auto Run() -> unsigned
		{
			return m_func(m_threadParam);
		}

		virtual auto Move(Thread& other) noexcept -> void
		{
			Close();
			m_func = std::move(other.m_func);
			m_status = other.m_status;
			m_threadHandle = std::move(other.m_threadHandle);
			m_threadParam = other.m_threadParam;
			m_started = std::move(other.m_started);
		}

		virtual auto InternalStart() -> void
		{
			// https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/beginthread-beginthreadex?view=vs-2019
			m_threadHandle = reinterpret_cast<Win32::HANDLE>(
				Win32::_beginthreadex(
					0,
					0,
					Thread::ThreadProc,
					this,
					0,
					nullptr
				)
			);
			if (not m_threadHandle)
			{
				int errorCode = 0;
				Win32::_get_errno(&errorCode);
				std::string errorMessage = std::format(
					"_beginthreadex() failed with error code {}", 
					errorCode
				);
				throw Error::Boring32Error(errorMessage);
			}
		}

		static auto ThreadProc(void* param) -> unsigned
		{
			Thread* threadObj = static_cast<Thread*>(param);
			if (not threadObj)
				throw Error::Boring32Error("threadObj is unexpectedly nullptr");

			unsigned returnCode = 0;
			threadObj->m_status = ThreadStatus::Running;

			threadObj->m_started.Signal();
			returnCode = threadObj->Run();
			threadObj->m_status = ThreadStatus::Finished;

			return returnCode;
		}

		ThreadStatus m_status = ThreadStatus::Ready;
		RAII::UniqueHandle m_threadHandle;
		void* m_threadParam = nullptr;
		std::function<int(void*)> m_func;
		ManualResetEvent m_started{ false, false };
	};
}