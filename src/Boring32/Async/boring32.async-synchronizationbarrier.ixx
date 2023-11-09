export module boring32.async:synchronizationbarrier;
import <string>;
import boring32.win32;
import boring32.error;

export namespace Boring32::Async
{
	class SynchronizationBarrier final
	{
		public:
			~SynchronizationBarrier()
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
				if (!Win32::InitializeSynchronizationBarrier(&m_barrier, m_totalThreads, m_spinCount))
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("InitializeSynchronizationBarrier() failed", lastError);
				}
				m_isInitialized = true;
			}

		public:
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
				if (!m_isInitialized)
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