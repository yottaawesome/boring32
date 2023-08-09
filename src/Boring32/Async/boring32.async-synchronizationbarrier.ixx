export module boring32.async:synchronizationbarrier;
import std;

import <win32.hpp>;
import boring32.error;

export namespace Boring32::Async
{
	class SynchronizationBarrier
	{
		public:
			virtual ~SynchronizationBarrier()
			{
				Close();
			}

			SynchronizationBarrier() = default;
			SynchronizationBarrier(const long totalThreads, const long spinCount)
				: m_totalThreads(totalThreads),
				m_spinCount(spinCount),
				m_isInitialized(false),
				m_barrier{ 0 }
			{
				//https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-initializesynchronizationbarrier
				if (!InitializeSynchronizationBarrier(&m_barrier, m_totalThreads, m_spinCount))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("InitializeSynchronizationBarrier() failed", lastError);
				}
				m_isInitialized = true;
			}

		public:
			virtual void Close()
			{
				if (m_isInitialized)
				{
					//https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-deletesynchronizationbarrier
					DeleteSynchronizationBarrier(&m_barrier);
					m_isInitialized = false;
				}
			}

			virtual bool Enter(const DWORD flags)
			{
				if (!m_isInitialized)
					throw Error::Boring32Error("Barrier is not initialised");
				//https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-entersynchronizationbarrier
				return EnterSynchronizationBarrier(&m_barrier, flags);
			}

		protected:
			long m_totalThreads = 0;
			long m_spinCount = 0;
			bool m_isInitialized = false;
			SYNCHRONIZATION_BARRIER m_barrier{ 0 };
	};
}