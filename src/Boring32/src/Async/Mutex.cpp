#include "pch.hpp"
#include <stdexcept>
#include "include/Error/Error.hpp"
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
			throw Error::Win32Error(__FUNCSIG__ ": failed to create mutex", GetLastError());
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
			m_name.size() > 0 ? m_name.c_str() : nullptr
		);
		m_mutex.SetInheritability(inheritable);
		if (m_mutex == nullptr)
			throw Error::Win32Error(__FUNCSIG__ ": failed to create mutex", GetLastError());

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
		if(m_name == L"")
			throw std::runtime_error(__FUNCSIG__ ": cannot open mutex with empty name");
		m_mutex = OpenMutexW(desiredAccess, isInheritable, m_name.c_str());
		if (m_mutex == nullptr)
			throw Error::Win32Error(__FUNCSIG__ ": failed to open mutex", GetLastError());
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

	bool Mutex::Lock(const DWORD waitTime, const bool isAlertable)
	{
		if (m_mutex == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": cannot wait on null mutex");

		DWORD result = WaitForSingleObjectEx(m_mutex.GetHandle(), waitTime, isAlertable);
		if (result == WAIT_FAILED)
			throw Error::Win32Error(__FUNCSIG__ ": failed to acquire mutex", GetLastError());
		if (result == WAIT_OBJECT_0)
			m_locked = true;
		if (result == WAIT_TIMEOUT)
			m_locked = false;
		return m_locked;
	}

	bool Mutex::Lock(const DWORD waitTime, const bool isAlertable, std::nothrow_t) noexcept
	{
		return Error::TryCatchLogToWCerr(
			[this, &waitTime, &isAlertable] { Lock(waitTime, isAlertable); }, 
			__FUNCSIG__
		);
	}

	void Mutex::Unlock()
	{
		if (m_mutex == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": cannot wait on null mutex");
		if (ReleaseMutex(m_mutex.GetHandle()) == false)
			throw Error::Win32Error(__FUNCSIG__ ": failed to release mutex", GetLastError());

		m_locked = false;
	}

	bool Mutex::Unlock(std::nothrow_t) noexcept
	{
		return Error::TryCatchLogToWCerr([this] { Unlock(); }, __FUNCSIG__);
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