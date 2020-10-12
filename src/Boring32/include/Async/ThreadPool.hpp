#pragma once
#include <Windows.h>

namespace Boring32::Async
{
	class ThreadPool
	{
		public:
			virtual ~ThreadPool();
			ThreadPool(const DWORD minThreads, const DWORD maxThreads);

		public:
			virtual void Close();

		protected:
			TP_POOL* m_pool;
			TP_CALLBACK_ENVIRON m_environ;
			DWORD m_minThreads;
			DWORD m_maxThreads;
	};
}
