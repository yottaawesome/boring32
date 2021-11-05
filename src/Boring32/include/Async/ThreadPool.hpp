#pragma once
#include <functional>
#include <Windows.h>
#include <memory>

namespace Boring32::Async
{
	// Doesn't work
	/*
	using ThreadPoolCallback = 
		std::function<
			 void(PTP_CALLBACK_INSTANCE instance, void* param, PTP_WORK work)
		>;
	*/

	typedef void
		(*ThreadPoolCallback)(
			PTP_CALLBACK_INSTANCE Instance,
			void*                 Parameter,
			PTP_WORK              Work
		);


	struct out {};

	class ThreadPool
	{
		public:
			using LambdaCallback = std::function<void(PTP_CALLBACK_INSTANCE Instance, void*, PTP_WORK)>;
			using WorkParamTuple = std::tuple<LambdaCallback&, void*>;
			struct WorkItem
			{
				LambdaCallback Callback;
				void* Parameter = nullptr;
				PTP_WORK Item = nullptr;
			};

			virtual ~ThreadPool();
			ThreadPool(const DWORD minThreads, const DWORD maxThreads);

		public:
			virtual DWORD GetMinThread() const noexcept final;
			virtual DWORD GetMaxThread() const noexcept final;

		public:
			virtual void SetMinAndMaxThreads(const DWORD min, const DWORD max);
			virtual void SetMaxThreads(const DWORD value);
			virtual void SetMinThreads(const DWORD value);
			virtual void Close();

			[[nodiscard("Return value should remain live until callback is fully completed")]]
			virtual PTP_WORK CreateWork(
				ThreadPoolCallback& callback,
				void* param
			);

			virtual void CreateWork(WorkItem& outWorkItem);
			virtual void CreateWork(WorkItem&& workItem) = delete;

			virtual void SubmitWork(PTP_WORK workItem);
			virtual void SetCallbackRunsLong();
			virtual std::shared_ptr<TP_POOL> GetPoolHandle() const noexcept final;

		protected:
			static void InternalCallback(
				PTP_CALLBACK_INSTANCE Instance,
				void* Parameter,
				PTP_WORK Work
			);
			static void ValidateArgs(const DWORD minThreads, const DWORD maxThreads);

		protected:
			std::shared_ptr<TP_POOL> m_pool;
			TP_CALLBACK_ENVIRON m_environ;
			DWORD m_minThreads;
			DWORD m_maxThreads;
	};
}
