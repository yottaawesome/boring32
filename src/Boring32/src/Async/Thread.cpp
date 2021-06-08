#include "pch.hpp"
#include <stdexcept>
#include <iostream>
#include "include/Error/Win32Error.hpp"
#include "include/Async/Thread.hpp"

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

	Thread::Thread(const Thread& other)
	{
		Copy(other);
	}

	Thread& Thread::operator=(const Thread& other)
	{
		Copy(other);
		return *this;
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
			std::string errorMessage = __FUNCSIG__ ": _beginthreadex() failed";
			// https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/get-errno?view=msvc-160
			errorMessage += _get_errno(&errorCode) == 0
				? "; error code: " + std::to_string(errorCode)
				: ", but could not determine the error code";

			throw std::runtime_error(errorMessage);
		}
	}

	ThreadStatus Thread::GetStatus() const noexcept
	{
		return m_status;
	}

	void Thread::Terminate(const DWORD exitCode)
	{
		if (m_threadHandle == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": no thread handle to terminate");
		if (TerminateThread(m_threadHandle.GetHandle(), exitCode) == false)
			throw Error::Win32Error(__FUNCSIG__ ": TerminateThread() failed", GetLastError());
		m_status = ThreadStatus::Terminated;
	}

	void Thread::Suspend()
	{
		if (m_threadHandle == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": no thread handle to suspend");
		if (m_status != ThreadStatus::Running)
			throw std::runtime_error(__FUNCSIG__ ": thread was not running when request to suspend occurred.");

		if (SuspendThread(m_threadHandle.GetHandle()) == false)
			throw Error::Win32Error(__FUNCSIG__ ": SuspendThread() failed", GetLastError());
		m_status = ThreadStatus::Suspended;
	}

	void Thread::Resume()
	{
		if (m_threadHandle == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": no thread handle to resume");
		if (m_status != ThreadStatus::Suspended)
			throw std::runtime_error(__FUNCSIG__ ": thread was not suspended when request to resume occurred.");

		if (ResumeThread(m_threadHandle.GetHandle()) == false)
			throw Error::Win32Error(__FUNCSIG__ ": ResumeThread() failed", GetLastError());
		m_status = ThreadStatus::Running;
	}

	bool Thread::Join(const DWORD waitTime)
	{
		if (m_threadHandle == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": no thread handle to wait on");

		const DWORD waitResult = WaitForSingleObject(m_threadHandle.GetHandle(), waitTime);
		if (waitResult == WAIT_OBJECT_0)
			return true;
		if (waitResult == WAIT_TIMEOUT)
			return false;
		if (waitResult == WAIT_ABANDONED)
			throw std::runtime_error(__FUNCSIG__ ": wait was abandoned");
		throw Error::Win32Error(__FUNCSIG__ ": WaitForSingleObject() failed", GetLastError());
	}

	UINT Thread::Run()
	{
		return m_func(m_threadParam);
	}

	UINT Thread::GetExitCode() const
	{
		if (m_threadHandle == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": no handle to thread; has the the thread been started or destroyed?");

		DWORD exitCode;
		if (GetExitCodeThread(m_threadHandle.GetHandle(), &exitCode) == false)
			throw Error::Win32Error(__FUNCSIG__ ": GetExitCodeThread() failed", GetLastError());
		return exitCode;
	}

	Raii::Win32Handle Thread::GetHandle() noexcept
	{
		return m_threadHandle;
	}
	
	bool Thread::WaitToStart(const DWORD millis)
	{
		return m_started.WaitOnEvent(millis, true);
	}

	UINT Thread::ThreadProc(void* param)
	{
		Thread* threadObj = static_cast<Thread*>(param);
		if (threadObj == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": threadObj is unexpectedly nullptr");

		UINT returnCode = 0;
		threadObj->m_status = ThreadStatus::Running;
		
		threadObj->m_started.Signal();
		returnCode = threadObj->Run();
		threadObj->m_status = ThreadStatus::Finished;

		return returnCode;
	}
}
