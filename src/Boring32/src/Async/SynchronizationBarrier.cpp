#include "pch.hpp"
#include "include/Error/Win32Error.hpp"
#include "include/Async/SynchronizationBarrier.hpp"

namespace Boring32::Async
{
	SynchronizationBarrier::~SynchronizationBarrier()
	{
		Close();
	}

	SynchronizationBarrier::SynchronizationBarrier()
	:	m_totalThreads(0),
		m_spinCount(0),
		m_isInitialized(false),
		m_barrier{ 0 }
	{ }

	SynchronizationBarrier::SynchronizationBarrier(const long totalThreads, const long spinCount)
	:	m_totalThreads(totalThreads),
		m_spinCount(spinCount),
		m_isInitialized(false),
		m_barrier{ 0 }
	{
		if (InitializeSynchronizationBarrier(&m_barrier, m_totalThreads, m_spinCount) == false)
			throw Error::Win32Error("SynchronizationBarrier::SynchronizationBarrier(): InitializeSynchronizationBarrier() failed", GetLastError());
		m_isInitialized = true;
	}

	void SynchronizationBarrier::Close()
	{
		if (m_isInitialized)
		{
			DeleteSynchronizationBarrier(&m_barrier);
			m_isInitialized = false;
		}
	}

	bool SynchronizationBarrier::Enter(const DWORD flags)
	{
		if (m_isInitialized == false)
			throw std::runtime_error("SynchronizationBarrier::Enter(): Barrier is no initialised");
		return EnterSynchronizationBarrier(&m_barrier, flags);
	}
}
