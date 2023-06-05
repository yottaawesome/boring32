module;

#include <source_location>;

export module boring32.async:threadpool;
import <functional>;
import <memory>;
import <source_location>;
import <tuple>;
import <iostream>;
import <win32.hpp>;
import boring32.error;

export namespace Boring32::Async::ThreadPools
{
	typedef void
		(*ThreadPoolCallback)(
			PTP_CALLBACK_INSTANCE Instance,
			void* Parameter,
			PTP_WORK              Work
		);

	class ThreadPool
	{
		public:
			using LambdaCallback = std::function<
				void(PTP_CALLBACK_INSTANCE Instance, void*, PTP_WORK)
			>;
			using WorkParamTuple = std::tuple<LambdaCallback&, void*>;

			template<typename T>
			struct WorkItem
			{
				using Lambda = std::function<
					void(PTP_CALLBACK_INSTANCE Instance, T, PTP_WORK)
				>;
				Lambda Callback;
				T Parameter = nullptr;
				PTP_WORK Item = nullptr;
			};

			// https://docs.microsoft.com/en-us/windows/win32/procthread/using-the-thread-pool-functions
			virtual ~ThreadPool()
			{
				Close();
			}

			ThreadPool() = default;

			ThreadPool(const DWORD minThreads, const DWORD maxThreads)
				: m_pool(nullptr),
				m_environ({ 0 }),
				m_minThreads(minThreads),
				m_maxThreads(maxThreads)
			{
				InternalCreate();
			}

		public:
			virtual DWORD GetMinThread() const noexcept final
			{
				return m_minThreads;
			}

			virtual DWORD GetMaxThread() const noexcept final
			{
				return m_maxThreads;
			}

		public:
			virtual void SetMinAndMaxThreads(const DWORD min, const DWORD max)
			{
				if (!m_pool)
					throw Error::Boring32Error("m_pool is nullptr");
				ValidateArgs(min, max);
				SetMinThreads(min);
				SetMaxThreads(max);
			}

			virtual void SetMaxThreads(const DWORD value)
			{
				if (!m_pool)
					throw Error::Boring32Error("m_pool is nullptr");
				ValidateArgs(m_minThreads, value);

				// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-setthreadpoolthreadmaximum
				m_maxThreads = value;
				SetThreadpoolThreadMaximum(m_pool.get(), m_maxThreads);
			}

			virtual void SetMinThreads(const DWORD value)
			{
				if (!m_pool)
					throw Error::Boring32Error("m_pool is nullptr");
				ValidateArgs(value, m_maxThreads);

				// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-setthreadpoolthreadminimum
				if (!SetThreadpoolThreadMinimum(m_pool.get(), value))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("SetThreadpoolThreadMinimum() failed", lastError);
				}

				m_minThreads = value;
			}

			virtual void Close()
			{
				if (m_pool)
				{
					// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-destroythreadpoolenvironment
					DestroyThreadpoolEnvironment(&m_environ);
					m_pool = nullptr;
				}
				m_environ = { 0 };
				m_minThreads = 0;
				m_maxThreads = 0;
			}

			[[nodiscard("Return value should remain live until callback is fully completed")]]
			virtual PTP_WORK CreateWork(
				ThreadPoolCallback& callback,
				void* param
			)
			{
				if (!m_pool)
					throw Error::Boring32Error("m_pool is nullptr");

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
				if (!item)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("CreateThreadPool() failed", lastError);
				}

				return item;
			}

			template<typename T>
			void CreateWork(WorkItem<T>& outWorkItem)
			{
				if (!m_pool)
					throw Error::Boring32Error("m_pool is nullptr");

				outWorkItem.Item = CreateThreadpoolWork(
					InternalCallback, 
					&outWorkItem, 
					&m_environ
				);
				if (!outWorkItem.Item)
				{
					const auto location = std::source_location::current();
					// using current() directly causes a compiler internal error
					const auto lastError = GetLastError();
					throw Error::Win32Error("CreateThreadpoolWork() failed", lastError);
				}
			}

			template<typename T>
			void CreateWork(WorkItem<T>&& workItem) = delete;

			virtual void SubmitWork(PTP_WORK workItem)
			{
				if (!m_pool)
					throw Error::Boring32Error("m_pool is nullptr");
				if (!workItem)
					throw Error::Boring32Error("workItem is nullptr");
				SubmitThreadpoolWork(workItem);
			}

			virtual void SetCallbackRunsLong()
			{
				//https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-setthreadpoolcallbackrunslong
				SetThreadpoolCallbackRunsLong(&m_environ);
			}

			virtual std::shared_ptr<TP_POOL> GetPoolHandle() const noexcept final
			{
				return m_pool;
			}

		protected:
			static void InternalCallback(
				PTP_CALLBACK_INSTANCE instance, 
				void* parameter, 
				PTP_WORK work
			)
			{
				// We cast this to a void, but the callback will know the correct type
				// This works because void* is 64 bits/8 bytes long, so it can encompass
				// all types, and we can avoid having to worry about passing and casting
				// void pointers around
				if (auto callback = reinterpret_cast<WorkItem<void*>*>(parameter)) try
				{
					callback->Callback(instance, callback->Parameter, work);
				}
				catch (const std::exception&)
				{
					// ICE
					//std::wcerr << ex.what() << std::endl;
				}
			}

			static void ValidateArgs(const DWORD minThreads, const DWORD maxThreads)
			{
				if (minThreads < 1)
					throw Error::Boring32Error("minThreads cannot be less than 1");
				if (maxThreads < 1)
					throw Error::Boring32Error("maxThreads cannot be less than 1");
				if (maxThreads < minThreads)
					throw Error::Boring32Error("maxThreads cannot be less than minThreads");
			}

			virtual ThreadPool& Copy(const ThreadPool& other)
			{
				Close();
				m_environ = other.m_environ;
				m_minThreads = other.m_minThreads;
				m_maxThreads = other.m_maxThreads;
				if (other.m_pool)
					InternalCreate();
				return *this;
			}

			virtual ThreadPool& Move(ThreadPool& other) noexcept
			{
				Close();
				m_environ = other.m_environ;
				m_minThreads = other.m_minThreads;
				m_maxThreads = other.m_maxThreads;
				m_pool = std::move(other.m_pool);
				return *this;
			}

			virtual void InternalCreate()
			{
				ValidateArgs(m_minThreads, m_maxThreads);

				// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-createthreadpool
				// https://docs.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-closethreadpool
				m_pool = std::shared_ptr<TP_POOL>(
					CreateThreadpool(nullptr), 
					CloseThreadpool
				);
				if (!m_pool)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("CreateThreadPool() failed", lastError);
				}

				SetMinAndMaxThreads(m_minThreads, m_maxThreads);
				// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-initializethreadpoolenvironment
				InitializeThreadpoolEnvironment(&m_environ);
				// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-setthreadpoolcallbackpool
				SetThreadpoolCallbackPool(&m_environ, m_pool.get());
			}

		protected:
			std::shared_ptr<TP_POOL> m_pool;
			TP_CALLBACK_ENVIRON m_environ{ 0 };
			DWORD m_minThreads = 0;
			DWORD m_maxThreads = 0;
	};
}
