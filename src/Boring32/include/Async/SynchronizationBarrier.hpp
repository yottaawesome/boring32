#pragma once
#include <Windows.h>

namespace Boring32::Async
{
	class SynchronizationBarrier
	{
		public:
			virtual ~SynchronizationBarrier();
			SynchronizationBarrier();
			SynchronizationBarrier(const long totalThreads, const long spinCount);

		public:
			virtual void Close();
			virtual bool Enter(const DWORD flags);

		protected:
			long m_totalThreads;
			long m_spinCount;
			bool m_isInitialized;
			SYNCHRONIZATION_BARRIER m_barrier;
	};
}
