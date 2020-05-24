#include "pch.hpp"
#include <stdexcept>
#include "include/Async/WaitableTimer.hpp"
#include "..\..\include\Async\Semaphore.hpp"

namespace Boring32::Async
{
	Semaphore::~Semaphore()
	{
		Close();
	}
	void Semaphore::Close()
	{
		m_handle.Close();
		m_handle = nullptr;
	}

	Semaphore::Semaphore()
	:	m_name(L""),
		m_currentCount(0),
		m_maxCount(0)
	{ }

	Semaphore::Semaphore(const bool createOrOpen, const std::wstring& name, const bool isInheritable, const long initialCount, const long maxCount)
	:	m_name(name),
		m_currentCount(initialCount),
		m_maxCount(maxCount)
	{
		const wchar_t* wcName = m_name == L"" ? nullptr : m_name.c_str();
		if (createOrOpen)
		{
			SECURITY_ATTRIBUTES sa{ 0 };
			sa.nLength = sizeof(sa);
			sa.bInheritHandle = isInheritable;
			m_handle = CreateSemaphore(
				&sa,
				initialCount,
				maxCount,
				wcName
			);
		}
		else
		{
			m_handle = OpenSemaphore(SEMAPHORE_ALL_ACCESS, isInheritable, wcName);
		}

		if (m_handle == nullptr)
			throw std::runtime_error("Failed to create or open semaphore");
	}

	Semaphore::Semaphore(const Semaphore& other)
	{
		Copy(other);
	}

	void Semaphore::operator=(const Semaphore& other)
	{
		Copy(other);
	}

	void Semaphore::Copy(const Semaphore& other)
	{
		Close();
		m_handle = other.m_handle;
		m_name = other.m_name;
		m_currentCount = other.m_currentCount;
		m_maxCount = other.m_maxCount;
	}

	Semaphore::Semaphore(Semaphore&& other) noexcept
	{
		Move(other);
	}

	void Semaphore::operator=(Semaphore&& other) noexcept
	{
		Move(other);
	}

	void Semaphore::Move(Semaphore& other) noexcept
	{
		Close();
		m_handle = std::move(other.m_handle);
		m_name = std::move(other.m_name);
		m_currentCount = other.m_currentCount;
		m_maxCount = other.m_maxCount;
	}

	void Semaphore::Release()
	{
		if (ReleaseSemaphore(m_handle.GetHandle(), 1, 0) == false)
			throw std::runtime_error("Failed to release semaphore");
		m_currentCount++;
	}

	void Semaphore::Release(const int countToRelease)
	{
		if (ReleaseSemaphore(m_handle.GetHandle(), 1, 0) == false)
			throw std::runtime_error("Failed to release semaphore");
		if((m_currentCount + countToRelease) > m_maxCount)
			throw std::runtime_error("Release count would exceed maximum");
		m_currentCount += countToRelease;
	}

	bool Semaphore::Acquire(const DWORD millisTimeout)
	{
		DWORD status = WaitForSingleObject(m_handle.GetHandle(), millisTimeout);
		if (status == WAIT_OBJECT_0)
		{
			m_currentCount--;
			return true;
		}

		if (status == WAIT_TIMEOUT)
			return false;
		if (status == WAIT_FAILED)
			throw std::runtime_error("WaitForSingleObject failed");
		if (status == WAIT_ABANDONED)
			throw std::runtime_error("The wait was abandoned");
		return false;
	}

	bool Semaphore::Acquire(const int countToAcquire, const DWORD millisTimeout)
	{
		if (countToAcquire > m_maxCount)
			throw std::runtime_error("Cannot acquire more than the maximum of the semaphore");
		
		int actualAcquired = 0;
		while (actualAcquired < countToAcquire)
		{
			if (Acquire(millisTimeout) == false)
			{
				if (actualAcquired > 0)
					Release(actualAcquired);
				return false;
			}
			actualAcquired++;
		}
		return true;
	}

	std::wstring Semaphore::GetName()
	{
		return m_name;
	}

	int Semaphore::GetCurrentCount()
	{
		return m_currentCount;
	}

	int Semaphore::GetMaxCount()
	{
		return m_maxCount;
	}

	HANDLE Semaphore::GetHandle()
	{
		return m_handle.GetHandle();
	}
}