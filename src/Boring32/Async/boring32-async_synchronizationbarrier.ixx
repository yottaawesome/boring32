export module boring32:async_synchronizationbarrier;
import boring32.win32;
import :error;

export namespace Boring32::Async
{
	struct SynchronizationBarrier final
	{
		~SynchronizationBarrier()
		{
			Close();
		}

		SynchronizationBarrier() = default;
		SynchronizationBarrier(const long totalThreads, const long spinCount)
			: m_totalThreads(totalThreads),
			m_spinCount(spinCount)
		{
			//https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-initializesynchronizationbarrier
			if (not Win32::InitializeSynchronizationBarrier(&m_barrier, m_totalThreads, m_spinCount))
				throw Error::Win32Error(Win32::GetLastError(), "InitializeSynchronizationBarrier() failed");
			m_isInitialized = true;
		}

		void Close()
		{
			if (m_isInitialized)
			{
				//https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-deletesynchronizationbarrier
				Win32::DeleteSynchronizationBarrier(&m_barrier);
				m_isInitialized = false;
			}
		}

		bool Enter(const Win32::DWORD flags)
		{
			if (not m_isInitialized)
				throw Error::Boring32Error("Barrier is not initialised");
			//https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-entersynchronizationbarrier
			return Win32::EnterSynchronizationBarrier(&m_barrier, flags);
		}

		private:
		long m_totalThreads = 0;
		long m_spinCount = 0;
		bool m_isInitialized = false;
		Win32::SYNCHRONIZATION_BARRIER m_barrier{ 0 };
	};
}