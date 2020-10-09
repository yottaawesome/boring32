#include "pch.hpp"
#include "include/Error/Win32Error.hpp"
#include "include/Async/ThreadPool.hpp"

namespace Boring32::Async
{
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
		
		// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-setthreadpoolthreadminimum
		SetThreadpoolThreadMinimum(m_pool, m_minThreads);
		// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-setthreadpoolthreadmaximum
		SetThreadpoolThreadMaximum(m_pool, m_maxThreads);
		// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-initializethreadpoolenvironment
		InitializeThreadpoolEnvironment(&m_environ);
		// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-setthreadpoolcallbackpool
		SetThreadpoolCallbackPool(&m_environ, m_pool);
	}
}