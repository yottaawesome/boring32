#include "pch.hpp"
#include <stdexcept>
#include "include/Async/Thread.hpp"

namespace Boring32::Async
{
	Thread::~Thread()
	{
		Close();
	}

	void Thread::Close()
	{
		if (this->m_hThread != nullptr)
		{
			m_hThread = nullptr;
			m_threadId = 0;
		}
	}

	Thread::Thread(void* param, bool destroyOnCompletion)
		: m_destroyOnCompletion(destroyOnCompletion),
		m_status(ThreadStatus::Ready),
		m_threadParam(param),
		m_threadId(0),
		m_hThread(nullptr, false),
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
		m_hThread = other.m_hThread;
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
		m_hThread = std::move(other.m_hThread);
		m_destroyOnCompletion = other.m_destroyOnCompletion;
		m_threadParam = other.m_threadParam;
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

	void Thread::Start(const std::function<int()>& func)
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

		TerminateThread(m_hThread.GetHandle(), (DWORD)ThreadStatus::Terminated);
	}

	void Thread::Suspend()
	{
		if (this->m_status != ThreadStatus::Running)
			throw std::runtime_error("Thread was not running when request to suspend occurred.");

		this->m_status = ThreadStatus::Suspended;
		SuspendThread(m_hThread.GetHandle());
	}

	void Thread::Resume()
	{
		if (this->m_status != ThreadStatus::Suspended)
			throw std::runtime_error("Thread was not suspended when request to resume occurred.");

		this->m_status = ThreadStatus::Running;
		ResumeThread(m_hThread.GetHandle());
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
