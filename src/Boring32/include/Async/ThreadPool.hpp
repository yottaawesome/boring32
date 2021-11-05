#pragma once
#include <functional>
#include <Windows.h>

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

	class ThreadPool
	{
		public:
			virtual ~ThreadPool();
			ThreadPool(const DWORD minThreads, const DWORD maxThreads);

		public:
			virtual DWORD GetMinThread() final;
			virtual DWORD GetMaxThread() final;

		public:
			virtual void SetMinAndMaxThreads(const DWORD min, const DWORD max);
			virtual void SetMaxThreads(const DWORD value);
			virtual void SetMinThreads(const DWORD value);
			virtual void Close();
			virtual PTP_WORK SubmitWork(
				ThreadPoolCallback& callback,
				void* param
			);

		protected:
			TP_POOL* m_pool;
			TP_CALLBACK_ENVIRON m_environ;
			DWORD m_minThreads;
			DWORD m_maxThreads;
	};
}
