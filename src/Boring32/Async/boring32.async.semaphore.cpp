module;

#include "pch.hpp"
#include <stdexcept>
#include <source_location>
#include <string>
#include <format>

module boring32.async.semaphore;
import boring32.error.win32error;

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
		const bool isInheritable,
		const ULONG initialCount,
		const ULONG maxCount
	)
	:	m_currentCount(initialCount),
		m_maxCount(maxCount)
	{
		InternalCreate(m_name, initialCount, maxCount, isInheritable);
	}

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
		if (m_name.empty())
			throw std::invalid_argument(__FUNCSIG__": cannot create named semaphore with empty string");
		InternalCreate(name, initialCount, maxCount, isInheritable);
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
		if (m_name.empty())
			throw std::invalid_argument(__FUNCSIG__": cannot open semaphore with empty string");
		if (maxCount == 0)
			throw std::invalid_argument(__FUNCSIG__": maxCount cannot be 0");
		//SEMAPHORE_ALL_ACCESS
		m_handle = OpenSemaphoreW(desiredAccess, isInheritable, m_name.c_str());
		if (m_handle == nullptr)
			throw Error::Win32Error(std::source_location::current(), "failed to open semaphore", GetLastError());
	}

	Semaphore::Semaphore(const Semaphore& other)
	:	m_currentCount(0),
		m_maxCount(0)
	{
		Copy(other);
	}

	void Semaphore::InternalCreate(
		const std::wstring& name,
		const ULONG initialCount,
		const ULONG maxCount,
		const bool isInheritable
	)
	{
		if (initialCount > maxCount)
			throw std::invalid_argument(__FUNCSIG__": initial count exceeds maximum count");
		if (maxCount == 0)
			throw std::invalid_argument(__FUNCSIG__": maxCount cannot be 0");
		m_handle = CreateSemaphoreW(
			nullptr,
			initialCount,
			maxCount,
			name.empty() ? nullptr : name.c_str()
		);
		if (m_handle == nullptr)
			throw Error::Win32Error(std::source_location::current(), "failed to create or open semaphore", GetLastError());

		m_handle.SetInheritability(isInheritable);
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

	long Semaphore::Release()
	{
		return Release(1);
	}

	long Semaphore::Release(const long countToRelease)
	{
		if (countToRelease == 0)
			return m_currentCount;
		if (m_handle == nullptr)
			throw std::runtime_error(__FUNCSIG__": m_handle is nullptr");

		long previousCount;
		if (!ReleaseSemaphore(m_handle.GetHandle(), countToRelease, &previousCount))
			throw Error::Win32Error(std::source_location::current(), "failed to release semaphore", GetLastError());
		m_currentCount += countToRelease;

		return previousCount;
	}

	bool Semaphore::Acquire(const DWORD millisTimeout)
	{
		return Acquire(millisTimeout, false);
	}
	
	bool Semaphore::Acquire(const DWORD millisTimeout, const bool isAlertable)
	{
		if (m_handle == nullptr)
			throw std::runtime_error(__FUNCSIG__": m_handle is nullptr");

		switch (const DWORD status = WaitForSingleObjectEx(*m_handle, millisTimeout, isAlertable))
		{
			case WAIT_OBJECT_0:
				m_currentCount--;
				return true;

			case WAIT_TIMEOUT:
				return false;

			case WAIT_ABANDONED:
				throw std::runtime_error(__FUNCSIG__": the wait was abandoned");

			case WAIT_FAILED:
				throw Error::Win32Error(std::source_location::current(), "WaitForSingleObject() failed", GetLastError());

			default:
				throw std::logic_error(std::format(__FUNCSIG__": unknown WaitForSingleObjectEx() value {}", status));
		}
	}

	bool Semaphore::AcquireMany(const long countToAcquire, const DWORD millisTimeout)
	{
		if (m_handle == nullptr)
			throw std::runtime_error(__FUNCSIG__": m_handle is nullptr");
		if (countToAcquire > m_maxCount)
			throw std::runtime_error(__FUNCSIG__": cannot acquire more than the maximum of the semaphore");
	
		for (int actualAcquired = 0; actualAcquired < countToAcquire; actualAcquired++)
		{
			if (!Acquire(millisTimeout))
			{
				Release(actualAcquired);
				return false;
			}
		}

		return true;
	}

	const std::wstring& Semaphore::GetName() const noexcept
	{
		return m_name;
	}

	long Semaphore::GetCurrentCount() const noexcept
	{
		return m_currentCount;
	}

	long Semaphore::GetMaxCount() const noexcept
	{
		return m_maxCount;
	}

	HANDLE Semaphore::GetHandle() const noexcept
	{
		return m_handle.GetHandle();
	}
}