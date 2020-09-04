#include "pch.hpp"
#include <stdexcept>
#include "include/Error/Win32Error.hpp"
#include "include/Async/Mutex.hpp"

namespace Boring32::Async
{
	Mutex::~Mutex()
	{
		Close();
	}

	void Mutex::Close()
	{
		if (m_mutex != nullptr)
		{
			if (m_locked)
				Unlock();
			m_mutex.Close();
		}
	}

	Mutex::Mutex()
	:	m_name(L""),
		m_created(false),
		m_mutex(nullptr),
		m_locked(false)
	{ }

	Mutex::Mutex(
		const std::wstring& name,
		const bool acquireOnCreation, 
		const bool inheritable
	)
	:	m_name(name),
		m_created(true),
		m_mutex(nullptr),
		m_locked(false)
	{
		m_mutex = CreateMutexW(
			nullptr,
			acquireOnCreation,
			m_name.size() > 0 ? m_name.c_str() : nullptr
		);
		m_mutex.SetInheritability(inheritable);
		if (m_mutex == nullptr)
			throw Error::Win32Error("Failed to create mutex", GetLastError());

		m_locked = acquireOnCreation;
	}

	Mutex::Mutex(
		const std::wstring& name,
		const DWORD desiredAccess,
		const bool isInheritable
	)
	:	m_name(name),
		m_created(false),
		m_mutex(nullptr),
		m_locked(false)
	{
		if(m_name == L"")
			throw std::runtime_error("Cannot open mutex with empty name");
		m_mutex = OpenMutexW(desiredAccess, isInheritable, m_name.c_str());
		if (m_mutex == nullptr)
			throw Error::Win32Error("Failed to open mutex", GetLastError());
	}

	Mutex::Mutex(const bool acquire, const bool inheritable)
	:	m_name(L""),
		m_created(false),
		m_locked(acquire),
		m_mutex(nullptr)
	{
		m_mutex = CreateMutexW(
			nullptr,
			m_locked,
			nullptr
		);
		m_mutex.SetInheritability(inheritable);
		if (m_mutex == nullptr)
			throw Error::Win32Error("Failed to create mutex", GetLastError());
	}

	Mutex::Mutex(const Mutex& other)
	{ 
		Copy(other);
	}

	Mutex& Mutex::operator=(const Mutex& other)
	{
		Copy(other);
		return *this;
	}

	void Mutex::Copy(const Mutex& other)
	{
		Close();
		m_name = other.m_name;
		m_created = false;
		m_locked = other.m_locked;
		m_mutex = other.m_mutex;
	}

	Mutex::Mutex(Mutex&& other) noexcept
	{
		Move(other);
	}

	Mutex& Mutex::operator=(Mutex&& other) noexcept
	{
		Move(other);
		return *this;
	}

	void Mutex::Move(Mutex& other) noexcept
	{
		m_name = std::move(other.m_name);
		m_created = other.m_created;
		m_locked = other.m_locked;
		m_mutex = std::move(other.m_mutex);
	}

	bool Mutex::Lock(const DWORD waitTime)
	{
		if (m_mutex == nullptr)
			throw std::runtime_error("Cannot wait on null mutex");

		DWORD result = WaitForSingleObject(m_mutex.GetHandle(), waitTime);
		if (result == WAIT_FAILED)
			throw Error::Win32Error("Failed to acquire mutex", GetLastError());
		if (result == WAIT_OBJECT_0)
			m_locked = true;
		if (result == WAIT_TIMEOUT)
			m_locked = false;
		return m_locked;
	}

	bool Mutex::SafeLock(const DWORD waitTime, const DWORD sleepTime)
	{
		if (waitTime == 0)
			throw std::runtime_error("SafeLock() requires a positive timeout period");

		while (Lock(waitTime) == false)
			Sleep(sleepTime);
		return true;
	}

	void Mutex::Unlock()
	{
		if (m_mutex == nullptr)
			throw std::runtime_error("Cannot wait on null mutex");
		if (ReleaseMutex(m_mutex.GetHandle()) == false)
			throw Error::Win32Error("Failed to release mutex", GetLastError());

		m_locked = false;
	}
}