#include "pch.hpp"
#include "include/Win32Utils.hpp"
#include <stdexcept>

namespace Win32Utils::Process
{
	Mutex::~Mutex()
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
		{
			throw std::runtime_error("Failed to create or open mutex");
		}

		m_locked = acquire;
	}

	void Mutex::Lock()
	{
		WaitForSingleObject(m_mutex, INFINITE);
		m_locked = true;
	}

	void Mutex::Unlock()
	{
		m_locked = false;
		if (!ReleaseMutex(m_mutex))
			throw std::runtime_error("Failed to release mutex");
	}
}