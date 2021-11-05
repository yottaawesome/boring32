#include "pch.hpp"
#include <tuple>
#include "include/Error/Win32Error.hpp"
#include "include/Async/ThreadPool.hpp"

namespace Boring32::Async
{
	using funcType = std::function<void(PTP_CALLBACK_INSTANCE Instance, void*, PTP_WORK)>;
	using WorkParamTuple = std::tuple<funcType&, void*>;

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
	}

	ThreadPool::ThreadPool(const DWORD minThreads, const DWORD maxThreads)
	:	m_pool(nullptr),
		m_environ({0}),
		m_minThreads(minThreads),
		m_maxThreads(maxThreads)
	{
		if (m_minThreads < 1)
			throw std::invalid_argument(__FUNCSIG__": minThreads cannot be less than 1");
		if (m_maxThreads < 1)
			throw std::invalid_argument(__FUNCSIG__": maxThreads cannot be less than 1");
		if (m_maxThreads < m_minThreads)
			throw std::invalid_argument(__FUNCSIG__": maxThreads cannot be less than minThreads");

		// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-createthreadpool
		// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-closethreadpool
		m_pool = std::shared_ptr<TP_POOL>(CreateThreadpool(nullptr), CloseThreadpool);
		if (m_pool == nullptr)
			throw Error::Win32Error(__FUNCSIG__": CreateThreadPool() failed", GetLastError());
		
		SetMinAndMaxThreads(minThreads, maxThreads);
		// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-initializethreadpoolenvironment
		InitializeThreadpoolEnvironment(&m_environ);
		// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-setthreadpoolcallbackpool
		SetThreadpoolCallbackPool(&m_environ, m_pool.get());
	}

	DWORD ThreadPool::GetMinThread() const
	{
		if (m_pool == nullptr)
			throw std::runtime_error(__FUNCSIG__": m_pool is nullptr");
		return m_minThreads;
	}

	DWORD ThreadPool::GetMaxThread() const
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

	void ThreadPool::SetMinThreads(const DWORD value)
	{
		if (m_pool == nullptr)
			throw std::runtime_error(__FUNCSIG__": m_pool is nullptr");
		if (value < 1)
			throw std::invalid_argument(__FUNCSIG__": value cannot be less than 1");
		if (value > m_maxThreads)
			throw std::invalid_argument(__FUNCSIG__": value cannot be less than minThreads");
		// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-setthreadpoolthreadminimum
		if (!SetThreadpoolThreadMinimum(m_pool.get(), value))
			throw Error::Win32Error(__FUNCSIG__": SetThreadpoolThreadMinimum() failed");
		m_minThreads = value;
	}

	void ThreadPool::SetMaxThreads(const DWORD value)
	{
		if (m_pool == nullptr)
			throw std::runtime_error(__FUNCSIG__": m_pool is nullptr");
		if (value < 1)
			throw std::invalid_argument(__FUNCSIG__": value cannot be less than 1");
		if (value < m_minThreads)
			throw std::invalid_argument(__FUNCSIG__": value cannot be less than minThreads");
		// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-setthreadpoolthreadmaximum
		m_maxThreads = value;
		SetThreadpoolThreadMaximum(m_pool.get(), m_maxThreads);
	}

	void InternalCallback(
		PTP_CALLBACK_INSTANCE Instance,
		void* Parameter,
		PTP_WORK Work
	)
	{
		if (Parameter == nullptr)
			return;

		std::unique_ptr<WorkParamTuple> tuple(reinterpret_cast<WorkParamTuple*>(Parameter));
		try
		{
			auto [unwrappedCallback, param] = *tuple;
			unwrappedCallback(Instance, param, Work);
		}
		catch (const std::exception& ex)
		{
			std::wcerr << ex.what() << std::endl;
		}
	}

	PTP_WORK ThreadPool::CreateWork(
		std::function<void(PTP_CALLBACK_INSTANCE Instance, void*, PTP_WORK)>& callback,
		void* param
	)
	{
		if (m_pool == nullptr)
			throw std::runtime_error(__FUNCSIG__": m_pool is nullptr");
		auto tuple = new WorkParamTuple(callback, param);

		const PTP_WORK item = CreateThreadpoolWork(
			InternalCallback,
			tuple,
			&m_environ
		);
		if (item == nullptr)
			throw Error::Win32Error(__FUNCSIG__": CreateThreadpoolWork() failed", GetLastError());

		return item;
	}

	PTP_WORK ThreadPool::CreateWork(
		ThreadPoolCallback& callback,
		const void* param
	)
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
			const_cast<void*>(param),
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
		if(workItem == nullptr)
			throw std::runtime_error(__FUNCSIG__": workItem is nullptr");
		SubmitThreadpoolWork(workItem);
	}
	
	void ThreadPool::SetCallbackRunsLong()
	{
		//https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-setthreadpoolcallbackrunslong
		SetThreadpoolCallbackRunsLong(&m_environ);
	}
}