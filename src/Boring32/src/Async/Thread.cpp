#include "pch.hpp"
#include <stdexcept>
#include "include/Async/Thread.hpp"

namespace Win32Utils::Async
{
	Thread::Thread(void* param, bool destroyOnCompletion)
		: m_destroyOnCompletion(destroyOnCompletion),
		m_status(ThreadStatus::Ready),
		m_threadParam(param),
		m_threadId(0),
		m_hThread(nullptr),
		m_returnCode(STILL_ACTIVE)
	{ }

	Thread::~Thread()
	{
		Cleanup();
	}

	void Thread::Cleanup()
	{
		if (this->m_hThread)
		{
			CloseHandle(m_hThread);
			m_hThread = nullptr;
		}
	}

	void Thread::Start()
	{
		m_hThread = (HANDLE)_beginthreadex(
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
		m_hThread = (HANDLE)_beginthreadex(
			0,
			0,
			Thread::ThreadProc,
			this,
			0,
			&m_threadId
		);
	}

	void Thread::Start(std::function<int()>* func)
	{
		m_func = *func;
		m_hThread = (HANDLE)_beginthreadex(
			0,
			0,
			Thread::ThreadProc,
			this,
			0,
			&m_threadId
		);
		if (m_hThread == nullptr)
		{
			m_status = ThreadStatus::Failure;
			throw std::runtime_error("Failed to start thread");
		}
	}

	void Thread::Start(std::function<int()> func)
	{
		m_func = func;
		m_hThread = (HANDLE)_beginthreadex(
			0,
			0,
			Thread::ThreadProc,
			this,
			0,
			&m_threadId
		);
		if (m_hThread == nullptr)
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

		TerminateThread(m_hThread, (DWORD)ThreadStatus::Terminated);
	}

	void Thread::Suspend()
	{
		if (this->m_status != ThreadStatus::Running)
			throw std::runtime_error("Thread was not running when request to suspend occurred.");

		this->m_status = ThreadStatus::Suspended;
		SuspendThread(m_hThread);
	}

	void Thread::Resume()
	{
		if (this->m_status != ThreadStatus::Suspended)
			throw std::runtime_error("Thread was not suspended when request to resume occurred.");

		this->m_status = ThreadStatus::Running;
		ResumeThread(m_hThread);
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

		threadObj->Cleanup();
		threadObj->m_returnCode = returnCode;
		if (threadObj->m_destroyOnCompletion)
			delete threadObj;

		return returnCode;
	}
}
