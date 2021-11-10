module;

#include "pch.hpp"
#include <tuple>
#include "include/Error/Win32Error.hpp"

module boring32.async.threadpools;

namespace Boring32::Async::ThreadPools
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
			// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-destroythreadpoolenvironment
			DestroyThreadpoolEnvironment(&m_environ);
			m_pool = nullptr;
		}
		m_environ = {0};
		m_minThreads = 0;
		m_maxThreads = 0;
	}

	ThreadPool::ThreadPool()
		: m_pool(nullptr),
		m_environ({ 0 }),
		m_minThreads(0),
		m_maxThreads(0)
	{};

	ThreadPool::ThreadPool(const DWORD minThreads, const DWORD maxThreads)
		: m_pool(nullptr),
		m_environ({ 0 }),
		m_minThreads(minThreads),
		m_maxThreads(maxThreads)
	{
		InternalCreate();
	}

	void ThreadPool::InternalCreate()
	{
		ValidateArgs(m_minThreads, m_maxThreads);

		// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-createthreadpool
		// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-closethreadpool
		m_pool = std::shared_ptr<TP_POOL>(CreateThreadpool(nullptr), CloseThreadpool);
		if (m_pool == nullptr)
			throw Error::Win32Error(__FUNCSIG__": CreateThreadPool() failed", GetLastError());

		SetMinAndMaxThreads(m_minThreads, m_maxThreads);
		// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-initializethreadpoolenvironment
		InitializeThreadpoolEnvironment(&m_environ);
		// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-setthreadpoolcallbackpool
		SetThreadpoolCallbackPool(&m_environ, m_pool.get());
	}


	DWORD ThreadPool::GetMinThread() const noexcept
	{
		return m_minThreads;
	}

	DWORD ThreadPool::GetMaxThread() const noexcept
	{
		return m_maxThreads;
	}

	void ThreadPool::SetMinAndMaxThreads(const DWORD min, const DWORD max)
	{
		if (m_pool == nullptr)
			throw std::runtime_error(__FUNCSIG__": m_pool is nullptr");
		ValidateArgs(min, max);
		SetMinThreads(min);
		SetMaxThreads(max);
	}

	void ThreadPool::SetMinThreads(const DWORD value)
	{
		if (m_pool == nullptr)
			throw std::runtime_error(__FUNCSIG__": m_pool is nullptr");
		ValidateArgs(value, m_maxThreads);

		// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-setthreadpoolthreadminimum
		if (!SetThreadpoolThreadMinimum(m_pool.get(), value))
			throw Error::Win32Error(__FUNCSIG__": SetThreadpoolThreadMinimum() failed");

		m_minThreads = value;
	}

	void ThreadPool::SetMaxThreads(const DWORD value)
	{
		if (m_pool == nullptr)
			throw std::runtime_error(__FUNCSIG__": m_pool is nullptr");
		ValidateArgs(m_minThreads, value);

		// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-setthreadpoolthreadmaximum
		m_maxThreads = value;
		SetThreadpoolThreadMaximum(m_pool.get(), m_maxThreads);
	}

	void ThreadPool::InternalCallback(PTP_CALLBACK_INSTANCE instance, void* parameter, PTP_WORK work)
	{
		// We cast this to a void, but the callback will know the correct type
		// This works because void* is 64 bits/8 bytes long, so it can encompass
		// all types, and we can avoid having to worry about passing and casting
		// void pointers around
		if (auto callback = reinterpret_cast<WorkItem<void*>*>(parameter)) try
		{
			callback->Callback(instance, callback->Parameter, work);
		}
		catch (const std::exception& ex)
		{
			std::wcerr << ex.what() << std::endl;
		}
	}

	PTP_WORK ThreadPool::CreateWork(ThreadPoolCallback& callback, void* param)
	{
		if (m_pool == nullptr)
			throw std::runtime_error(__FUNCSIG__": m_pool is nullptr");

		// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-createthreadpoolwork
		// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-submitthreadpoolwork
		// https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms687396(v=vs.85)
		// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-closethreadpoolwork
		// https://docs.microsoft.com/en-us/archive/msdn-magazine/2011/august/windows-with-c-the-windows-thread-pool-and-work
		const PTP_WORK item = CreateThreadpoolWork(
			callback,
			param,
			&m_environ
		);
		if (item == nullptr)
			throw Error::Win32Error(__FUNCSIG__": CreateThreadPool() failed", GetLastError());

		return item;
	}

	void ThreadPool::SubmitWork(PTP_WORK workItem)
	{
		if (m_pool == nullptr)
			throw std::runtime_error(__FUNCSIG__": m_pool is nullptr");
		if (workItem == nullptr)
			throw std::runtime_error(__FUNCSIG__": workItem is nullptr");
		SubmitThreadpoolWork(workItem);
	}
	
	void ThreadPool::SetCallbackRunsLong()
	{
		//https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-setthreadpoolcallbackrunslong
		SetThreadpoolCallbackRunsLong(&m_environ);
	}

	std::shared_ptr<TP_POOL> ThreadPool::GetPoolHandle() const noexcept
	{
		return m_pool;
	}

	void ThreadPool::ValidateArgs(const DWORD minThreads, const DWORD maxThreads)
	{
		if (minThreads < 1)
			throw std::invalid_argument(__FUNCSIG__": minThreads cannot be less than 1");
		if (maxThreads < 1)
			throw std::invalid_argument(__FUNCSIG__": maxThreads cannot be less than 1");
		if (maxThreads < minThreads)
			throw std::invalid_argument(__FUNCSIG__": maxThreads cannot be less than minThreads");
	}

	ThreadPool& ThreadPool::Copy(const ThreadPool& other)
	{
		Close();
		m_environ = other.m_environ;
		m_minThreads = other.m_minThreads;
		m_maxThreads = other.m_maxThreads;
		if (other.m_pool)
			InternalCreate();
		return *this;
	}

	ThreadPool& ThreadPool::Move(ThreadPool& other) noexcept
	{
		Close();
		m_environ = other.m_environ;
		m_minThreads = other.m_minThreads;
		m_maxThreads = other.m_maxThreads;
		m_pool = std::move(other.m_pool);
		return *this;
	}
}