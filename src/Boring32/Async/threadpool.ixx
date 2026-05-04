export module boring32:async.threadpool;
import std;
import :win32;
import :error;

export namespace Boring32::Async::ThreadPools
{
	typedef void (*ThreadPoolCallback)(Win32::PTP_CALLBACK_INSTANCE, void*, Win32::PTP_WORK);

	class ThreadPool final
	{
	public:
		using LambdaCallback = std::function<void(Win32::PTP_CALLBACK_INSTANCE Instance, void*, Win32::PTP_WORK)>;
		using WorkParamTuple = std::tuple<LambdaCallback&, void*>;

		template<typename T>
		struct WorkItem
		{
			using Lambda = std::function<
				void(Win32::PTP_CALLBACK_INSTANCE Instance, T, Win32::PTP_WORK)
			>;
			Lambda Callback;
			T Parameter = nullptr;
			Win32::PTP_WORK Item = nullptr;
		};

		// https://docs.microsoft.com/en-us/windows/win32/procthread/using-the-thread-pool-functions
		~ThreadPool()
		{
			Close();
		}

		ThreadPool() = default;

		ThreadPool(const Win32::DWORD minThreads, const Win32::DWORD maxThreads)
			: m_pool(nullptr),
			m_environ({ 0 }),
			m_minThreads(minThreads),
			m_maxThreads(maxThreads)
		{
			InternalCreate();
		}

		auto GetMinThread() const noexcept -> Win32::DWORD
		{
			return m_minThreads;
		}

		auto GetMaxThread() const noexcept -> Win32::DWORD
		{
			return m_maxThreads;
		}

		auto SetMinAndMaxThreads(const Win32::DWORD min, const Win32::DWORD max) -> void
		{
			if (not m_pool)
				throw Error::Boring32Error("m_pool is nullptr");
			ValidateArgs(min, max);
			SetMinThreads(min);
			SetMaxThreads(max);
		}

		auto SetMaxThreads(const Win32::DWORD value) -> void
		{
			if (not m_pool)
				throw Error::Boring32Error("m_pool is nullptr");
			ValidateArgs(m_minThreads, value);

			// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-setthreadpoolthreadmaximum
			m_maxThreads = value;
			Win32::SetThreadpoolThreadMaximum(m_pool.get(), m_maxThreads);
		}

		auto SetMinThreads(const Win32::DWORD value) -> void
		{
			if (not m_pool)
				throw Error::Boring32Error("m_pool is nullptr");
			ValidateArgs(value, m_maxThreads);

			// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-setthreadpoolthreadminimum
			if (not Win32::SetThreadpoolThreadMinimum(m_pool.get(), value))
				throw Error::Win32Error{Win32::GetLastError(), "SetThreadpoolThreadMinimum() failed"};

			m_minThreads = value;
		}

		auto Close() -> void
		{
			if (m_pool)
			{
				// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-destroythreadpoolenvironment
				Win32::DestroyThreadpoolEnvironment(&m_environ);
				m_pool = nullptr;
			}
			m_environ = { 0 };
			m_minThreads = 0;
			m_maxThreads = 0;
		}

		[[nodiscard("Return value should remain live until callback is fully completed")]]
		auto CreateWork(ThreadPoolCallback& callback, void* param) -> Win32::PTP_WORK
		{
			if (not m_pool)
				throw Error::Boring32Error("m_pool is nullptr");

			// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-createthreadpoolwork
			// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-submitthreadpoolwork
			// https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms687396(v=vs.85)
			// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-closethreadpoolwork
			// https://docs.microsoft.com/en-us/archive/msdn-magazine/2011/august/windows-with-c-the-windows-thread-pool-and-work
			Win32::PTP_WORK item = Win32::CreateThreadpoolWork(callback, param, &m_environ);
			if (not item)
				throw Error::Win32Error{Win32::GetLastError(), "CreateThreadPool() failed"};

			return item;
		}

		template<typename T>
		auto CreateWork(WorkItem<T>& outWorkItem) -> void
		{
			if (not m_pool)
				throw Error::Boring32Error("m_pool is nullptr");

			outWorkItem.Item = Win32::CreateThreadpoolWork(
				InternalCallback, 
				&outWorkItem, 
				&m_environ
			);
			if (not outWorkItem.Item)
				throw Error::Win32Error{Win32::GetLastError(), "CreateThreadpoolWork() failed"};
		}

		template<typename T>
		auto CreateWork(WorkItem<T>&& workItem) -> void = delete;

		auto SubmitWork(Win32::PTP_WORK workItem) -> void
		{
			if (not m_pool)
				throw Error::Boring32Error("m_pool is nullptr");
			if (not workItem)
				throw Error::Boring32Error("workItem is nullptr");
			Win32::SubmitThreadpoolWork(workItem);
		}

		auto SetCallbackRunsLong() -> void
		{
			//https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-setthreadpoolcallbackrunslong
			Win32::SetThreadpoolCallbackRunsLong(&m_environ);
		}

		auto GetPoolHandle() const noexcept -> std::shared_ptr<Win32::TP_POOL>
		{
			return m_pool;
		}

	protected:
		static auto InternalCallback(
			Win32::PTP_CALLBACK_INSTANCE instance,
			void* parameter, 
			Win32::PTP_WORK work
		) -> void
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
				// ICE
				std::wcerr << ex.what() << std::endl;
			}
		}

		static auto ValidateArgs(const Win32::DWORD minThreads, const Win32::DWORD maxThreads) -> void
		{
			if (minThreads < 1)
				throw Error::Boring32Error("minThreads cannot be less than 1");
			if (maxThreads < 1)
				throw Error::Boring32Error("maxThreads cannot be less than 1");
			if (maxThreads < minThreads)
				throw Error::Boring32Error("maxThreads cannot be less than minThreads");
		}

		auto Copy(const ThreadPool& other) -> ThreadPool&
		{
			Close();
			m_environ = other.m_environ;
			m_minThreads = other.m_minThreads;
			m_maxThreads = other.m_maxThreads;
			if (other.m_pool)
				InternalCreate();
			return *this;
		}

		auto Move(ThreadPool& other) noexcept -> ThreadPool&
		{
			Close();
			m_environ = other.m_environ;
			m_minThreads = other.m_minThreads;
			m_maxThreads = other.m_maxThreads;
			m_pool = std::move(other.m_pool);
			return *this;
		}

		auto InternalCreate() -> void
		{
			ValidateArgs(m_minThreads, m_maxThreads);

			// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-createthreadpool
			// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-closethreadpool
			m_pool = std::shared_ptr<Win32::TP_POOL>(
				Win32::CreateThreadpool(nullptr), 
				Win32::CloseThreadpool
			);
			if (not m_pool)
				throw Error::Win32Error{Win32::GetLastError(), "CreateThreadPool() failed"};

			SetMinAndMaxThreads(m_minThreads, m_maxThreads);
			// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-initializethreadpoolenvironment
			Win32::InitializeThreadpoolEnvironment(&m_environ);
			// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-setthreadpoolcallbackpool
			Win32::SetThreadpoolCallbackPool(&m_environ, m_pool.get());
		}

		std::shared_ptr<Win32::TP_POOL> m_pool;
		Win32::TP_CALLBACK_ENVIRON m_environ{ 0 };
		Win32::DWORD m_minThreads = 0;
		Win32::DWORD m_maxThreads = 0;
	};
}
