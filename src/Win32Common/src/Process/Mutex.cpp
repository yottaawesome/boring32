#include "pch.hpp"
#include "include/Win32Utils.hpp"
#include <stdexcept>

namespace Win32Utils::Process
{
	Mutex::~Mutex()
	{
		Cleanup();
	}

	void Mutex::Cleanup()
	{
		if (m_mutex != nullptr)
		{
			CloseHandle(m_mutex);
			m_mutex = nullptr;
		}
	}

	Mutex::Mutex(const std::wstring_view name, const bool create, const bool acquire, const bool inheritable)
	:	m_name(name),
		m_inheritable(inheritable),
		m_created(create),
		m_mutex(nullptr),
		m_locked(false)
	{
		if (m_created)
		{
			SECURITY_ATTRIBUTES lp{ 0 };
			lp.nLength = sizeof(lp);
			lp.bInheritHandle = inheritable;
			m_mutex = CreateMutex(
				&lp,
				acquire,
				m_name.c_str()
			);
		}
		else
		{
			m_mutex = OpenMutex(SYNCHRONIZE, inheritable, m_name.c_str());
		}

		if (m_mutex == nullptr)
			throw std::runtime_error("Failed to create or open mutex");

		m_locked = create && acquire;
	}

	Mutex::Mutex(const bool acquire, const bool inheritable)
	:	m_name(L""),
		m_inheritable(inheritable),
		m_created(false),
		m_locked(acquire),
		m_mutex(nullptr)
	{
		SECURITY_ATTRIBUTES lp{ 0 };
		lp.nLength = sizeof(lp);
		lp.bInheritHandle = m_inheritable;
		m_mutex = CreateMutex(
			&lp,
			m_locked,
			nullptr
		);
		if (m_mutex == nullptr)
			throw std::runtime_error("Failed to create or open mutex");
	}

	Mutex::Mutex(const Mutex& other)
	:	m_name(other.m_name),
		m_inheritable(other.m_inheritable),
		m_created(false),
		m_locked(other.m_locked),
		m_mutex(nullptr)
	{
		if (other.m_mutex == nullptr)
			throw std::runtime_error("Other mutex is not in a valid state for assignment.");
		bool succeeded = DuplicateHandle(
			GetCurrentProcess(),
			other.m_mutex,
			GetCurrentProcess(),
			&m_mutex,
			0,
			other.m_inheritable,
			DUPLICATE_SAME_ACCESS
		);
		if (succeeded == false)
			throw std::runtime_error("Failed to duplicated handle.");
	}


	void Mutex::operator=(const Mutex& other)
	{
		if (other.m_mutex == nullptr)
			throw std::runtime_error("Other mutex is not in a valid state for assignment.");

		Cleanup();
		bool succeeded = DuplicateHandle(
			GetCurrentProcess(),
			other.m_mutex,
			GetCurrentProcess(),
			&m_mutex,
			0,
			other.m_inheritable,
			DUPLICATE_SAME_ACCESS
		);
		if (succeeded == false)
			throw std::runtime_error("Failed to duplicated handle.");
	}

	bool Mutex::Lock(const DWORD waitTime)
	{
		DWORD result = WaitForSingleObject(m_mutex, waitTime);
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

	void Mutex::Unlock()
	{
		m_locked = false;
		if (!ReleaseMutex(m_mutex))
			throw std::runtime_error("Failed to release mutex");
	}
}