module;

#include <source_location>;

export module boring32.async:thread;
import <functional>;
import <iostream>;
import <string>;
import <win32.hpp>;
import :event;
import boring32.error;
import boring32.raii;

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

	class Thread
	{
		// Default constructible, movable, not copyable
		public:
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

		public:
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

		public:
			/// <summary>
			///		Terminates the thread. Be careful when using this
			///		function, as it prevents proper clean up of the 
			///		thread's objects and may leave shared objects in  
			///		an inconsistent state. Note also that if a thread
			///		is waiting on a kernel object, it will not be 
			///		terminated until the wait is finished.
			/// </summary>
			virtual void Terminate(const DWORD exitCode)
			{
				if (!m_threadHandle)
					throw Error::Boring32Error("No thread handle to terminate");
				if (!TerminateThread(m_threadHandle.GetHandle(), exitCode))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("TerminateThread() failed", lastError);
				}
				m_status = ThreadStatus::Terminated;
			}

			virtual void Suspend()
			{
				if (!m_threadHandle)
					throw Error::Boring32Error("No thread handle to suspend");
				if (m_status != ThreadStatus::Running)
					throw Error::Boring32Error("Thread was not running when request to suspend occurred.");

				if (!SuspendThread(m_threadHandle.GetHandle()))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("SuspendThread() failed", lastError);
				}
				m_status = ThreadStatus::Suspended;
			}

			virtual void Resume()
			{
				if (!m_threadHandle)
					throw Error::Boring32Error("No thread handle to resume");
				if (m_status != ThreadStatus::Suspended)
					throw Error::Boring32Error("Thread was not suspended when request to resume occurred.");

				if (!ResumeThread(m_threadHandle.GetHandle()))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("ResumeThread() failed", lastError);
				}
				m_status = ThreadStatus::Running;
			}

			virtual bool Join(const DWORD waitTime)
			{
				if (m_threadHandle == nullptr)
					throw Error::Boring32Error("No thread handle to wait on");

				const DWORD waitResult = WaitForSingleObject(m_threadHandle.GetHandle(), waitTime);
				if (waitResult == WAIT_OBJECT_0)
					return true;
				if (waitResult == WAIT_TIMEOUT)
					return false;
				if (waitResult == WAIT_ABANDONED)
					throw Error::Boring32Error("Wait was abandoned");
				const auto lastError = GetLastError();
				throw Error::Win32Error("WaitForSingleObject() failed", lastError);
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

			virtual UINT GetExitCode() const
			{
				if (!m_threadHandle)
					throw Error::Boring32Error("No handle to thread; has the the thread been started or destroyed?");

				DWORD exitCode;
				if (!GetExitCodeThread(m_threadHandle.GetHandle(), &exitCode))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("GetExitCodeThread() failed", lastError);
				}
				return exitCode;
			}

			virtual RAII::Win32Handle GetHandle() noexcept
			{
				return m_threadHandle;
			}

			virtual bool WaitToStart(const DWORD millis)
			{
				return m_started.WaitOnEvent(millis, true);
			}

			virtual void SetDescription(const std::wstring& description)
			{
				if (!m_threadHandle)
					throw Error::Boring32Error("No thread created.");

				const HRESULT hr = SetThreadDescription(
					m_threadHandle.GetHandle(),
					description.c_str()
				);
				if (FAILED(hr))
					throw Error::COMError("SetThreadDescription() failed", hr);
			}

			virtual std::wstring GetDescription()
			{
				if (!m_threadHandle)
					throw Error::Boring32Error("No thread created.");

				wchar_t* pThreadDescription = nullptr;
				// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getthreaddescription
				const HRESULT hr = GetThreadDescription(
					m_threadHandle,
					&pThreadDescription
				);
				if (FAILED(hr))
					throw Error::COMError("GetThreadDescription() failed", hr);
				if (!pThreadDescription)
					return {};
				RAII::LocalHeapUniquePtr<wchar_t> deleter(pThreadDescription);
				return pThreadDescription;
			}

		protected:
			virtual UINT Run()
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
				m_threadHandle = (HANDLE)_beginthreadex(
					0,
					0,
					Thread::ThreadProc,
					this,
					0,
					nullptr
				);
				if (!m_threadHandle)
				{
					int errorCode = 0;
					_get_errno(&errorCode);
					std::string errorMessage = std::format(
						"_beginthreadex() failed with error code {}", 
						errorCode
					);
					throw Error::Boring32Error(errorMessage);
				}
			}

			static UINT WINAPI ThreadProc(void* param)
			{
				Thread* threadObj = static_cast<Thread*>(param);
				if (threadObj == nullptr)
					throw Error::Boring32Error("threadObj is unexpectedly nullptr");

				UINT returnCode = 0;
				threadObj->m_status = ThreadStatus::Running;

				threadObj->m_started.Signal();
				returnCode = threadObj->Run();
				threadObj->m_status = ThreadStatus::Finished;

				return returnCode;
			}

		protected:
			ThreadStatus m_status = ThreadStatus::Ready;
			RAII::Win32Handle m_threadHandle;
			void* m_threadParam = nullptr;
			std::function<int(void*)> m_func;
			Event m_started{ false, true, false, L"" };
	};
}