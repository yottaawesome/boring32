#include "pch.hpp"
#include <stdexcept>
#include "include/Error/Win32Exception.hpp"
#include "include/Async/Thread.hpp"

namespace Boring32::Async
{
	Thread::~Thread()
	{
		Close();
	}

	void Thread::Close()
	{
		if (m_thread != nullptr)
		{
			m_thread = nullptr;
			m_threadId = 0;
		}
	}

	Thread::Thread(void* param, bool destroyOnCompletion)
		: m_destroyOnCompletion(destroyOnCompletion),
		m_status(ThreadStatus::Ready),
		m_threadParam(param),
		m_threadId(0),
		m_thread(nullptr),
		m_returnCode(STILL_ACTIVE)
	{ }

	Thread::Thread(const Thread& other)
	{
		Copy(other);
	}

	void Thread::operator=(const Thread& other)
	{
		Copy(other);
	}

	void Thread::Copy(const Thread& other)
	{
		Close();
		m_func = other.m_func;
		m_status = other.m_status;
		m_returnCode = other.m_returnCode;
		m_threadId = other.m_threadId;
		m_thread = other.m_thread;
		m_destroyOnCompletion = other.m_destroyOnCompletion;
		m_threadParam = other.m_threadParam;
	}

	Thread::Thread(const Thread&& other) noexcept
	{
		Copy(other);
	}

	void Thread::operator=(Thread&& other) noexcept
	{
		Copy(other);
	}

	void Thread::Copy(Thread& other) noexcept
	{
		Close();
		m_func = std::move(other.m_func);
		m_status = other.m_status;
		m_returnCode = other.m_returnCode;
		m_threadId = other.m_threadId;
		m_thread = std::move(other.m_thread);
		m_destroyOnCompletion = other.m_destroyOnCompletion;
		m_threadParam = other.m_threadParam;
	}

	void Thread::Start()
	{
		m_thread = (HANDLE)_beginthreadex(
			0,
			0,
			Thread::ThreadProc,
			this,
			0,
			&m_threadId
		);
	}

	void Thread::Start(int(*simpleFunc)())
	{
		m_func = simpleFunc;
		// https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/beginthread-beginthreadex?view=vs-2019
		m_thread = (HANDLE)_beginthreadex(
			0,
			0,
			Thread::ThreadProc,
			this,
			0,
			&m_threadId
		);
	}

	void Thread::Start(const std::function<int()>& func)
	{
		m_func = func;
		m_thread = (HANDLE)_beginthreadex(
			0,
			0,
			Thread::ThreadProc,
			this,
			0,
			&m_threadId
		);
		if (m_thread == nullptr)
		{
			m_status = ThreadStatus::Failure;
			throw std::runtime_error("Failed to start thread");
		}
	}

	ThreadStatus Thread::GetStatus()
	{
		return m_status;
	}

	void Thread::Terminate()
	{
		if (this->m_status != ThreadStatus::Running)
			throw std::runtime_error("Thread was not running when request to terminate occurred.");

		if (TerminateThread(m_thread.GetHandle(), (DWORD)ThreadStatus::Terminated) == false)
			throw Error::Win32Exception("Thread::Suspend(): TerminateThread() failed", GetLastError());
	}

	void Thread::Suspend()
	{
		if (this->m_status != ThreadStatus::Running)
			throw std::runtime_error("Thread was not running when request to suspend occurred.");

		this->m_status = ThreadStatus::Suspended;
		if (SuspendThread(m_thread.GetHandle()) == false)
			throw Error::Win32Exception("Thread::Suspend(): SuspendThread() failed", GetLastError());
	}

	void Thread::Resume()
	{
		if (this->m_status != ThreadStatus::Suspended)
			throw std::runtime_error("Thread was not suspended when request to resume occurred.");

		this->m_status = ThreadStatus::Running;
		if (ResumeThread(m_thread.GetHandle()) == false)
			throw Error::Win32Exception("Thread::Suspend(): ResumeThread() failed", GetLastError());
	}

	bool Thread::Join(const DWORD waitTime)
	{
		if (this->m_status != ThreadStatus::Running)
			throw std::runtime_error("Thread was not running when request to join occurred.");

		const DWORD waitResult = WaitForSingleObject(m_thread.GetHandle(), waitTime);
		if(waitResult == WAIT_OBJECT_0)
			return true;
		if (waitResult == WAIT_TIMEOUT)
			return false;
		if (waitResult == WAIT_ABANDONED)
			throw std::runtime_error("Thread::Join(): wait was abandoned");
		throw Error::Win32Exception("Thread::Join(): WaitForSingleObject() failed", GetLastError());
	}

	UINT Thread::Run()
	{
		return m_func();
	}

	UINT Thread::GetReturnCode()
	{
		return m_returnCode;
	}

	UINT Thread::ThreadProc(void* param)
	{
		Thread* threadObj = (Thread*)param;

		UINT returnCode = 0;
		threadObj->m_status = ThreadStatus::Running;
		try
		{
			returnCode = threadObj->Run();
			threadObj->m_status = ThreadStatus::Finished;
		}
		catch (const std::exception& ex)
		{
			returnCode = 1;
			threadObj->m_status = ThreadStatus::FinishedWithError;
		}

		threadObj->Close();
		threadObj->m_returnCode = returnCode;
		if (threadObj->m_destroyOnCompletion)
			delete threadObj;

		return returnCode;
	}
}
