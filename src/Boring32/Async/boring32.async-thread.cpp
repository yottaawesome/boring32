module;

#include <source_location>

module boring32.async:thread;
import boring32.error;
import <iostream>;

namespace Boring32::Async
{
	Thread::~Thread()
	{
		Close();
	}

	void Thread::Close()
	{
		m_threadHandle = nullptr;
	}

	Thread::Thread()
	:	
		m_status(ThreadStatus::Ready),
		m_threadParam(nullptr),
		m_threadHandle(nullptr),
		m_started(false, true, false, L"")
	{ }

	Thread::Thread(void* param)
	:	Thread()
	{ 
		m_threadParam = param;
	}

	void Thread::Copy(const Thread& other)
	{
		Close();
		m_func = other.m_func;
		m_status = other.m_status;
		m_threadHandle = other.m_threadHandle;
		m_threadParam = other.m_threadParam;
		m_started = other.m_started;
	}

	Thread::Thread(Thread&& other) noexcept
	{
		Move(other);
	}

	Thread& Thread::operator=(Thread&& other) noexcept
	{
		Move(other);
		return *this;
	}

	void Thread::Move(Thread& other) noexcept
	{
		Close();
		m_func = std::move(other.m_func);
		m_status = other.m_status;
		m_threadHandle = std::move(other.m_threadHandle);
		m_threadParam = other.m_threadParam;
		m_started = std::move(other.m_started);
	}

	bool Thread::operator==(const ThreadStatus status) const noexcept
	{
		return m_status == status;
	}

	void Thread::Start()
	{
		InternalStart();
	}

	void Thread::Start(int(*simpleFunc)(void*))
	{
		m_func = simpleFunc;
		InternalStart();
	}

	void Thread::Start(const std::function<int(void*)>& func)
	{
		m_func = func;
		InternalStart();
	}

	void Thread::InternalStart()
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
		if (m_threadHandle == nullptr)
		{
			int errorCode = 0;
			std::string errorMessage = "_beginthreadex() failed";
			// https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/get-errno?view=msvc-160
			errorMessage += _get_errno(&errorCode) == 0
				? "; error code: " + std::to_string(errorCode)
				: ", but could not determine the error code";

			throw Error::Boring32Error(errorMessage);
		}
	}

	ThreadStatus Thread::GetStatus() const noexcept
	{
		return m_status;
	}

	void Thread::Terminate(const DWORD exitCode)
	{
		if (m_threadHandle == nullptr)
			throw Error::Boring32Error("No thread handle to terminate");
		if (TerminateThread(m_threadHandle.GetHandle(), exitCode) == false)
			throw Error::Win32Error("TerminateThread() failed", GetLastError());
		m_status = ThreadStatus::Terminated;
	}

	void Thread::Suspend()
	{
		if (m_threadHandle == nullptr)
			throw Error::Boring32Error("No thread handle to suspend");
		if (m_status != ThreadStatus::Running)
			throw Error::Boring32Error("Thread was not running when request to suspend occurred.");

		if (SuspendThread(m_threadHandle.GetHandle()) == false)
			throw Error::Win32Error("SuspendThread() failed", GetLastError());
		m_status = ThreadStatus::Suspended;
	}

	void Thread::Resume()
	{
		if (m_threadHandle == nullptr)
			throw Error::Boring32Error("No thread handle to resume");
		if (m_status != ThreadStatus::Suspended)
			throw Error::Boring32Error("Thread was not suspended when request to resume occurred.");

		if (ResumeThread(m_threadHandle.GetHandle()) == false)
			throw Error::Win32Error("ResumeThread() failed", GetLastError());
		m_status = ThreadStatus::Running;
	}

	bool Thread::Join(const DWORD waitTime)
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
		throw Error::Win32Error("WaitForSingleObject() failed", GetLastError());
	}

	UINT Thread::Run()
	{
		return m_func(m_threadParam);
	}

	UINT Thread::GetExitCode() const
	{
		if (m_threadHandle == nullptr)
			throw Error::Boring32Error("No handle to thread; has the the thread been started or destroyed?");

		DWORD exitCode;
		if (GetExitCodeThread(m_threadHandle.GetHandle(), &exitCode) == false)
			throw Error::Win32Error("GetExitCodeThread() failed", GetLastError());
		return exitCode;
	}

	RAII::Win32Handle Thread::GetHandle() noexcept
	{
		return m_threadHandle;
	}
	
	bool Thread::WaitToStart(const DWORD millis)
	{
		return m_started.WaitOnEvent(millis, true);
	}
	
	void Thread::SetDescription(const std::wstring& description)
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
	
	std::wstring Thread::GetDescription()
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

	UINT Thread::ThreadProc(void* param)
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
}
