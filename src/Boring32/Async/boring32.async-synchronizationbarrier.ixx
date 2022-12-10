export module boring32.async:synchronizationbarrier;
import <win32.hpp>;

export namespace Boring32::Async
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