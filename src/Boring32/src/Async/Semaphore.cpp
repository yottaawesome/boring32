#include "pch.hpp"
#include <stdexcept>
#include "include/Error/Win32Error.hpp"
#include "include/Async/Semaphore.hpp"

namespace Boring32::Async
{
	Semaphore::~Semaphore()
	{
		Close();
	}

	void Semaphore::Close()
	{
		m_handle = nullptr;
	}

	Semaphore::Semaphore()
	:	m_name(L""),
		m_currentCount(0),
		m_maxCount(0)
	{ }

	Semaphore::Semaphore(
		std::wstring name, 
		const bool isInheritable, 
		const ULONG initialCount,
		const ULONG maxCount
	)
	:	m_name(std::move(name)),
		m_currentCount(initialCount),
		m_maxCount(maxCount)
	{
		if (initialCount > maxCount)
			throw std::invalid_argument(__FUNCSIG__": initial count exceeds maximum count");
		m_handle = CreateSemaphoreW(
			nullptr,
			initialCount,
			maxCount,
			m_name == L"" ? nullptr : m_name.c_str()
		);
		if (m_handle == nullptr)
			throw Error::Win32Error(__FUNCSIG__": failed to open semaphore", GetLastError());

		m_handle.SetInheritability(isInheritable);
	}

	Semaphore::Semaphore(
		std::wstring name,
		const bool isInheritable,
		const long initialCount,
		const long maxCount,
		const DWORD desiredAccess
	)
	:	m_name(std::move(name)),
		m_currentCount(initialCount),
		m_maxCount(maxCount)
	{
		if (initialCount > maxCount)
			throw std::invalid_argument(__FUNCSIG__": initial count exceeds maximum count");
		if (m_name == L"")
			throw std::runtime_error(__FUNCSIG__": cannot open mutex with empty string");
		//SEMAPHORE_ALL_ACCESS
		m_handle = OpenSemaphoreW(desiredAccess, isInheritable, m_name.c_str());
		if (m_handle == nullptr)
			throw Error::Win32Error(__FUNCSIG__": failed to open semaphore", GetLastError());
	}

	Semaphore::Semaphore(const Semaphore& other)
	:	m_currentCount(0),
		m_maxCount(0)
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
		m_currentCount = other.m_currentCount.load();
		m_maxCount = other.m_maxCount;
	}

	Semaphore::Semaphore(Semaphore&& other) noexcept
	:	m_currentCount(0),
		m_maxCount(0)
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
		m_currentCount = other.m_currentCount.load();
		m_maxCount = other.m_maxCount;
	}

	void Semaphore::Release()
	{
		if (m_handle == nullptr)
			throw std::runtime_error(__FUNCSIG__": m_handle is nullptr");
		if (ReleaseSemaphore(m_handle.GetHandle(), 1, 0) == false)
			throw Error::Win32Error(__FUNCSIG__": failed to release semaphore", GetLastError());
		m_currentCount++;
	}

	void Semaphore::Release(const int countToRelease)
	{
		if (m_handle == nullptr)
			throw std::runtime_error(__FUNCSIG__": m_handle is nullptr");
		if (ReleaseSemaphore(m_handle.GetHandle(), 1, 0) == false)
			throw Error::Win32Error(__FUNCSIG__": failed to release semaphore", GetLastError());
		m_currentCount -= countToRelease;
	}

	bool Semaphore::Acquire(const DWORD millisTimeout)
	{
		if (m_handle == nullptr)
			throw std::runtime_error(__FUNCSIG__": m_handle is nullptr");
		const DWORD status = WaitForSingleObject(m_handle.GetHandle(), millisTimeout);
		if (status == WAIT_OBJECT_0)
		{
			m_currentCount--;
			return true;
		}
		if (status == WAIT_TIMEOUT)
			return false;
		if (status == WAIT_ABANDONED)
			throw std::runtime_error(__FUNCSIG__": the wait was abandoned");
		if (status == WAIT_FAILED)
			throw Error::Win32Error(__FUNCSIG__": WaitForSingleObject() failed", GetLastError());
		return false;
	}

	bool Semaphore::Acquire(const int countToAcquire, const DWORD millisTimeout)
	{
		if (m_handle == nullptr)
			throw std::runtime_error(__FUNCSIG__": m_handle is nullptr");
		if (countToAcquire > m_maxCount)
			throw std::runtime_error(__FUNCSIG__": cannot acquire more than the maximum of the semaphore");
		
		int actualAcquired = 0;
		while (actualAcquired < countToAcquire)
		{
			if (!Acquire(millisTimeout))
			{
				if (actualAcquired > 0)
					Release(actualAcquired);
				return false;
			}
			actualAcquired++;
		}
		return true;
	}

	const std::wstring& Semaphore::GetName() const noexcept
	{
		return m_name;
	}

	int Semaphore::GetCurrentCount() const noexcept
	{
		return m_currentCount;
	}

	int Semaphore::GetMaxCount() const noexcept
	{
		return m_maxCount;
	}

	HANDLE Semaphore::GetHandle() const noexcept
	{
		return m_handle.GetHandle();
	}
}