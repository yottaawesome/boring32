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
		m_threadId = 0;
	}

	Thread::Thread()
	:	m_destroyOnCompletion(false),
		m_status(ThreadStatus::Ready),
		m_threadParam(nullptr),
		m_threadId(0),
		m_threadHandle(nullptr),
		m_returnCode(STILL_ACTIVE),
		m_started(false, true, false, L"")
	{ }

	Thread::Thread(void* param, bool destroyOnCompletion)
	:	m_destroyOnCompletion(destroyOnCompletion),
		m_status(ThreadStatus::Ready),
		m_threadParam(param),
		m_threadId(0),
		m_threadHandle(nullptr),
		m_returnCode(STILL_ACTIVE),
		m_started(false, true, false, L"")
	{ }

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
		m_returnCode = other.m_returnCode;
		m_threadId = other.m_threadId;
		m_threadHandle = other.m_threadHandle;
		m_destroyOnCompletion = other.m_destroyOnCompletion;
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
		m_returnCode = other.m_returnCode;
		m_threadId = other.m_threadId;
		m_threadHandle = std::move(other.m_threadHandle);
		m_destroyOnCompletion = other.m_destroyOnCompletion;
		m_threadParam = other.m_threadParam;
		m_started = std::move(other.m_started);
	}

	void Thread::Start()
	{
		InternalStart();
	}

	void Thread::Start(int(*simpleFunc)())
	{
		m_func = simpleFunc;
		InternalStart();
	}

	void Thread::Start(const std::function<int()>& func)
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
			&m_threadId
		);
		if (m_threadHandle == nullptr)
		{
			int errorCode = 0;
			std::string errorMessage = "Thread::Start(): _beginthreadex() failed";
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

	void Thread::Terminate()
	{
		if (m_threadHandle == nullptr)
			throw std::runtime_error("No thread handle to suspend");
		if (TerminateThread(m_threadHandle.GetHandle(), (DWORD)ThreadStatus::Terminated) == false)
			throw Error::Win32Error("Thread::Suspend(): TerminateThread() failed", GetLastError());
	}

	void Thread::Suspend()
	{
		if (m_threadHandle == nullptr)
			throw std::runtime_error("No thread handle to suspend");
		if (m_status != ThreadStatus::Running)
			throw std::runtime_error("Thread was not running when request to suspend occurred.");

		this->m_status = ThreadStatus::Suspended;
		if (SuspendThread(m_threadHandle.GetHandle()) == false)
			throw Error::Win32Error("Thread::Suspend(): SuspendThread() failed", GetLastError());
	}

	void Thread::Resume()
	{
		if (m_threadHandle == nullptr)
			throw std::runtime_error("No thread handle to resume");
		if (this->m_status != ThreadStatus::Suspended)
			throw std::runtime_error("Thread was not suspended when request to resume occurred.");

		this->m_status = ThreadStatus::Running;
		if (ResumeThread(m_threadHandle.GetHandle()) == false)
			throw Error::Win32Error("Thread::Suspend(): ResumeThread() failed", GetLastError());
	}

	bool Thread::Join(const DWORD waitTime)
	{
		if (m_threadHandle == nullptr)
			throw std::runtime_error("No thread handle to wait on");

		const DWORD waitResult = WaitForSingleObject(m_threadHandle.GetHandle(), waitTime);
		if (waitResult == WAIT_OBJECT_0)
			return true;
		if (waitResult == WAIT_TIMEOUT)
			return false;
		if (waitResult == WAIT_ABANDONED)
			throw std::runtime_error("Thread::Join(): wait was abandoned");
		throw Error::Win32Error("Thread::Join(): WaitForSingleObject() failed", GetLastError());
	}

	UINT Thread::Run()
	{
		return m_func();
	}

	UINT Thread::GetExitCode() const noexcept
	{
		return m_returnCode;
	}

	Raii::Win32Handle Thread::GetHandle()
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
			throw std::runtime_error("Thread::ThreadProc(): threadObj is unexpectedly nullptr");

		UINT returnCode = 0;
		threadObj->m_status = ThreadStatus::Running;
		
		threadObj->m_started.Signal();
		returnCode = threadObj->Run();
		threadObj->m_status = ThreadStatus::Finished;

		threadObj->m_returnCode = returnCode;
		if (threadObj->m_destroyOnCompletion)
			delete threadObj;

		return returnCode;
	}
}
