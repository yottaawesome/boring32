#include "pch.hpp"
#include <stdexcept>
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
		m_mutex(nullptr, false),
		m_locked(false)
	{ }

	Mutex::Mutex(
		const std::wstring_view name, 
		const bool createNew, 
		const bool acquireOnCreation, 
		const bool inheritable
	)
	:	m_name(name),
		m_created(createNew),
		m_mutex(nullptr, false),
		m_locked(false)
	{
		if (m_created)
		{
			SECURITY_ATTRIBUTES lp{ 0 };
			lp.nLength = sizeof(lp);
			lp.bInheritHandle = inheritable;
			m_mutex = CreateMutex(
				&lp,
				acquireOnCreation,
				m_name.size() > 0 ? m_name.c_str() : nullptr
			);
		}
		else
		{
			m_mutex = OpenMutex(SYNCHRONIZE, inheritable, m_name.c_str());
		}

		if (m_mutex == nullptr)
			throw std::runtime_error("Failed to create or open mutex");

		m_mutex = inheritable;
		m_locked = createNew && acquireOnCreation;
	}

	Mutex::Mutex(const bool acquire, const bool inheritable)
	:	m_name(L""),
		m_created(false),
		m_locked(acquire),
		m_mutex(nullptr, false)
	{
		SECURITY_ATTRIBUTES lp{ 0 };
		lp.nLength = sizeof(lp);
		lp.bInheritHandle = inheritable;
		m_mutex = CreateMutex(
			&lp,
			m_locked,
			nullptr
		);
		if (m_mutex == nullptr)
			throw std::runtime_error("Failed to create or open mutex");
		m_mutex = inheritable;
	}

	Mutex::Mutex(const Mutex& other)
	{ 
		Copy(other);
	}

	void Mutex::operator=(const Mutex& other)
	{
		Copy(other);
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

	void Mutex::operator=(Mutex&& other) noexcept
	{
		Move(other);
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
		if (result == WAIT_OBJECT_0)
		{
			m_locked = true;
			return true;
		}
		if (result == WAIT_TIMEOUT)
		{
			m_locked = false;
			return false;
		}
		throw std::runtime_error("Failed to acquire mutex.");
	}

	bool Mutex::SafeLock(const DWORD waitTime)
	{
		if (waitTime == 0)
			throw std::runtime_error("SafeLock() requires a positive timeout period");

		while (Lock(waitTime) == false)
			;
	}

	void Mutex::Unlock()
	{
		if (m_mutex == nullptr)
			throw std::runtime_error("Cannot wait on null mutex");
		if (!ReleaseMutex(m_mutex.GetHandle()))
			throw std::runtime_error("Failed to release mutex");

		m_locked = false;
	}
}