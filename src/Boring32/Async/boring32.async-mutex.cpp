module;

#include <stdexcept>
#include <string>
#include <iostream>
#include <format>
#include <source_location>
#include <Windows.h>

module boring32.async:mutex;
import boring32.error;
import :functions;

namespace Boring32::Async
{
	Mutex::~Mutex()
	{
		Close();
	}

	void Mutex::Close()
	{
		if (m_mutex)
		{
			if (m_locked)
				Unlock();
			m_mutex.Close();
		}
	}

	Mutex::Mutex()
	:	m_created(false),
		m_mutex(nullptr),
		m_locked(false)
	{ }

	Mutex::Mutex(const bool acquire, const bool inheritable)
	:	m_created(false),
		m_locked(acquire),
		m_mutex(nullptr)
	{
		m_mutex = CreateMutexW(
			nullptr,
			m_locked,
			nullptr
		);
		m_mutex.SetInheritability(inheritable);
		if (!m_mutex)
			throw Error::Win32Error(std::source_location::current(), "Failed to create mutex", GetLastError());
	}

	Mutex::Mutex(
		const bool acquireOnCreation, 
		const bool inheritable,
		std::wstring name
	)
	:	m_name(std::move(name)),
		m_created(true),
		m_mutex(nullptr),
		m_locked(false)
	{
		m_mutex = CreateMutexW(
			nullptr,
			acquireOnCreation,
			m_name.empty() ? nullptr : m_name.c_str()
		);
		m_mutex.SetInheritability(inheritable);
		if (!m_mutex)
			throw Error::Win32Error(std::source_location::current(), "Failed to create mutex", GetLastError());

		m_locked = acquireOnCreation;
	}

	Mutex::Mutex(
		const bool acquireOnOpen,
		const bool isInheritable,
		std::wstring name,
		const DWORD desiredAccess
	)
	:	m_name(name),
		m_created(false),
		m_mutex(nullptr),
		m_locked(false)
	{
		if(m_name.empty())
			throw Error::Boring32Error("Cannot open mutex with empty name");
		m_mutex = OpenMutexW(desiredAccess, isInheritable, m_name.c_str());
		if (!m_mutex)
			throw Error::Win32Error(std::source_location::current(), "failed to open mutex", GetLastError());
		if(acquireOnOpen)
			Lock(INFINITE, true);
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
	
	bool Mutex::Lock()
	{
		return Lock(INFINITE, false);
	}
	
	bool Mutex::Lock(const DWORD waitTime)
	{
		return Lock(waitTime, false);
	}
	
	bool Mutex::Lock(const bool isAlertable)
	{
		return Lock(INFINITE, isAlertable);
	}

	bool Mutex::Lock(const DWORD waitTime, const bool isAlertable)
	{
		if (!m_mutex)
			throw Error::Boring32Error("Cannot wait on null mutex");
		m_locked = WaitFor(m_mutex.GetHandle(), waitTime, isAlertable);
		return m_locked;
	}

	bool Mutex::Lock(const DWORD waitTime, const bool isAlertable, const std::nothrow_t&) noexcept try
	{
		return Lock(waitTime, isAlertable);
	}
	catch (const std::exception& ex)
	{
		std::wcerr << std::format("{}: Lock() failed: {}\n", __FUNCSIG__, ex.what()).c_str();
		return false;
	}

	void Mutex::Unlock()
	{
		if (!m_mutex)
			throw Error::Boring32Error("Cannot wait on null mutex");
		if (!ReleaseMutex(m_mutex.GetHandle()))
			throw Error::Win32Error(std::source_location::current(), "Failed to release mutex", GetLastError());

		m_locked = false;
	}

	bool Mutex::Unlock(const std::nothrow_t&) noexcept try
	{
		Unlock();
		return true;
	}
	catch (const std::exception& ex)
	{
		std::wcerr << std::format("{}: Unlock() failed: {}\n", __FUNCSIG__, ex.what()).c_str();
		return false;
	}

	HANDLE Mutex::GetHandle() const noexcept
	{
		return m_mutex.GetHandle();
	}

	const std::wstring& Mutex::GetName() const noexcept
	{
		return m_name;
	}
}