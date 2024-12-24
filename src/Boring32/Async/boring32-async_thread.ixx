export module boring32:async_thread;
import boring32.shared;
import :error;
import :raii;
import :async_event;

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

		virtual Thread& operator=(const Thread&) = delete;

		virtual Thread& operator=(Thread&& other) noexcept
		{
			Move(other);
			return *this;
		}

		virtual bool operator==(const ThreadStatus status) const noexcept
		{
			return m_status == status;
		}

		///	Terminates the thread. Be careful when using this
		///	function, as it prevents proper clean up of the 
		///	thread's objects and may leave shared objects in  
		///	an inconsistent state. Note also that if a thread
		///	is waiting on a kernel object, it will not be 
		///	terminated until the wait is finished.
		virtual void Terminate(const Win32::DWORD exitCode)
		{
			if (not m_threadHandle)
				throw Error::Boring32Error("No thread handle to terminate");
			if (not Win32::TerminateThread(m_threadHandle.GetHandle(), exitCode))
				throw Error::Win32Error(Win32::GetLastError(), "TerminateThread() failed");
			m_status = ThreadStatus::Terminated;
		}

		virtual void Suspend()
		{
			if (not m_threadHandle)
				throw Error::Boring32Error("No thread handle to suspend");
			if (m_status != ThreadStatus::Running)
				throw Error::Boring32Error("Thread was not running when request to suspend occurred.");

			if (not Win32::SuspendThread(m_threadHandle.GetHandle()))
				throw Error::Win32Error(Win32::GetLastError(), "SuspendThread() failed");
			m_status = ThreadStatus::Suspended;
		}

		virtual void Resume()
		{
			if (not m_threadHandle)
				throw Error::Boring32Error("No thread handle to resume");
			if (m_status != ThreadStatus::Suspended)
				throw Error::Boring32Error("Thread was not suspended when request to resume occurred.");
			if (not Win32::ResumeThread(m_threadHandle.GetHandle()))
				throw Error::Win32Error(Win32::GetLastError(), "ResumeThread() failed");
			m_status = ThreadStatus::Running;
		}

		virtual bool Join(const Win32::DWORD waitTime)
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
			throw Error::Win32Error(Win32::GetLastError(), "WaitForSingleObject() failed");
		}

		virtual void Close()
		{
			m_threadHandle = nullptr;
		}
			
		virtual void Start()
		{
			InternalStart();
		}

		virtual void Start(int(*simpleFunc)(void*))
		{
			m_func = simpleFunc;
			InternalStart();
		}

		virtual void Start(const std::function<int(void*)>& func)
		{
			m_func = func;
			InternalStart();
		}

		virtual ThreadStatus GetStatus() const noexcept
		{
			return m_status;
		}

		virtual unsigned long GetExitCode() const
		{
			if (not m_threadHandle)
				throw Error::Boring32Error("No handle to thread; has the the thread been started or destroyed?");

			Win32::DWORD exitCode;
			if (not Win32::GetExitCodeThread(m_threadHandle.GetHandle(), &exitCode))
				throw Error::Win32Error(Win32::GetLastError(), "GetExitCodeThread() failed");
			return exitCode;
		}

		virtual RAII::Win32Handle GetHandle() noexcept
		{
			return m_threadHandle;
		}

		virtual bool WaitToStart(const Win32::DWORD millis)
		{
			return m_started.WaitOnEvent(millis, true);
		}

		virtual void SetDescription(const std::wstring& description)
		{
			if (not m_threadHandle)
				throw Error::Boring32Error("No thread created.");

			Win32::HRESULT hr = Win32::SetThreadDescription(
				m_threadHandle.GetHandle(),
				description.c_str()
			);
			if (Win32::HrFailed(hr))
				throw Error::COMError("SetThreadDescription() failed", hr);
		}

		virtual std::wstring GetDescription()
		{
			if (not m_threadHandle)
				throw Error::Boring32Error("No thread created.");

			wchar_t* pThreadDescription = nullptr;
			// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getthreaddescription
			Win32::HRESULT hr = Win32::GetThreadDescription(m_threadHandle, &pThreadDescription);
			if (Win32::HrFailed(hr))
				throw Error::COMError("GetThreadDescription() failed", hr);
			if (not pThreadDescription)
				return {};
			RAII::LocalHeapUniquePtr<wchar_t> deleter(pThreadDescription);
			return pThreadDescription;
		}

		protected:
		virtual unsigned Run()
		{
			return m_func(m_threadParam);
		}

		virtual void Copy(const Thread& other)
		{
			Close();
			m_func = other.m_func;
			m_status = other.m_status;
			m_threadHandle = other.m_threadHandle;
			m_threadParam = other.m_threadParam;
			m_started = other.m_started;
		}

		virtual void Move(Thread& other) noexcept
		{
			Close();
			m_func = std::move(other.m_func);
			m_status = other.m_status;
			m_threadHandle = std::move(other.m_threadHandle);
			m_threadParam = other.m_threadParam;
			m_started = std::move(other.m_started);
		}

		virtual void InternalStart()
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

		static unsigned __stdcall ThreadProc(void* param)
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
		RAII::Win32Handle m_threadHandle;
		void* m_threadParam = nullptr;
		std::function<int(void*)> m_func;
		ManualResetEvent m_started{ false, false };
	};
}