#include "pch.hpp"
#include "include/Error/Win32Error.hpp"
#include "include/Async/ThreadPool.hpp"

namespace Boring32::Async
{
	// https://docs.microsoft.com/en-us/windows/win32/procthread/using-the-thread-pool-functions
	ThreadPool::~ThreadPool()
	{
		Close();
	}

	void ThreadPool::Close()
	{
		if (m_pool)
		{
			// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-closethreadpool
			CloseThreadpool(m_pool);
			m_pool = nullptr;
			// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-destroythreadpoolenvironment
			DestroyThreadpoolEnvironment(&m_environ);
		}
	}

	ThreadPool::ThreadPool(const DWORD minThreads, const DWORD maxThreads)
	:	m_pool(nullptr),
		m_environ({0}),
		m_minThreads(minThreads),
		m_maxThreads(maxThreads)
	{
		if (m_minThreads < 1 || m_maxThreads < m_minThreads)
			throw std::invalid_argument("Invalid minThreads or maxThreads specified");

		// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-createthreadpool
		m_pool = CreateThreadpool(nullptr);
		if (m_pool == nullptr)
			throw Error::Win32Error("ThreadPool::ThreadPool(): CreateThreadPool() failed", GetLastError());
		
		// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-initializethreadpoolenvironment
		InitializeThreadpoolEnvironment(&m_environ);
		// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-setthreadpoolcallbackpool
		SetThreadpoolCallbackPool(&m_environ, m_pool);
	}

	DWORD ThreadPool::GetMinThread()
	{
		if (m_pool == nullptr)
			throw std::runtime_error(__FUNCSIG__": m_pool is nullptr");
		return m_minThreads;
	}

	DWORD ThreadPool::GetMaxThread()
	{
		if (m_pool == nullptr)
			throw std::runtime_error(__FUNCSIG__": m_pool is nullptr");
		return m_maxThreads;
	}

	void ThreadPool::SetMinAndMaxThreads(const DWORD min, const DWORD max)
	{
		if (m_pool == nullptr)
			throw std::runtime_error(__FUNCSIG__": m_pool is nullptr");
		SetMinThreads(min);
		SetMaxThreads(max);
	}

	void ThreadPool::SetMaxThreads(const DWORD value)
	{
		if (m_pool == nullptr)
			throw std::runtime_error(__FUNCSIG__": m_pool is nullptr");
		// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-setthreadpoolthreadminimum
		if (!SetThreadpoolThreadMinimum(m_pool, m_minThreads))
			throw Error::Win32Error(__FUNCSIG__": SetThreadpoolThreadMinimum() failed");
	}
	void ThreadPool::SetMinThreads(const DWORD value)
	{
		if (m_pool == nullptr)
			throw std::runtime_error(__FUNCSIG__": m_pool is nullptr");
		// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-setthreadpoolthreadmaximum
		SetThreadpoolThreadMaximum(m_pool, m_maxThreads)
	}

	PTP_WORK ThreadPool::SubmitWork(
		ThreadPoolCallback& callback,
		void* param
	)
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-createthreadpoolwork
		// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-submitthreadpoolwork
		// https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms687396(v=vs.85)
		// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-closethreadpoolwork
		// https://docs.microsoft.com/en-us/archive/msdn-magazine/2011/august/windows-with-c-the-windows-thread-pool-and-work
		PTP_WORK item = CreateThreadpoolWork(
			callback,
			param,
			&m_environ
		);
		if(item == nullptr)
			throw Error::Win32Error("ThreadPool::ThreadPool(): CreateThreadPool() failed", GetLastError());
		return item;
	}
}