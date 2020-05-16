#include "pch.hpp"
#include <stdexcept>
#include "include/Raii/Win32Handle.hpp"

namespace Win32Utils::Raii
{
	Win32Handle::Win32Handle()
	:	m_handle(nullptr),
		m_inheritable(false)
	{ }
	
	Win32Handle::Win32Handle(const Win32Handle& otherHandle)
	:	m_handle(nullptr),
		m_inheritable(otherHandle.m_inheritable)
	{
		Copy(otherHandle);
	}

	void Win32Handle::operator=(const Win32Handle& other)
	{
		Copy(other);
	}

	void Win32Handle::Copy(const Win32Handle& other)
	{
		Close();
		m_inheritable = other.IsInheritable();
		if (other != nullptr)
			m_handle = DuplicatePassedHandle(other.GetHandle(), other.IsInheritable());
	}

	Win32Handle::Win32Handle(Win32Handle&& otherHandle) noexcept
	{
		m_inheritable = otherHandle.m_inheritable;
		m_handle = otherHandle.m_handle;
		otherHandle.m_inheritable = false;
		otherHandle.m_handle = nullptr;
	}

	void Win32Handle::operator=(Win32Handle&& other) noexcept
	{
		Close();
		m_inheritable = other.m_inheritable;
		m_handle = other.m_handle;
		other.m_handle = nullptr;
		other.m_inheritable = false;
	}

	Win32Handle::Win32Handle(const HANDLE handle, const bool inheritable)
	:	m_handle(handle),
		m_inheritable(inheritable)
	{ }

	Win32Handle::~Win32Handle()
	{
		Close();
	}

	HANDLE Win32Handle::GetHandle() const
	{
		return m_handle;
	}

	HANDLE& Win32Handle::GetHandleAddress()
	{
		return m_handle;
	}

	void Win32Handle::Close()
	{
		if (m_handle)
		{
			CloseHandle(m_handle);
			m_handle = nullptr;
		}
	}

	void Win32Handle::operator=(const HANDLE other)
	{
		Close();
		m_handle = other;
	}

	bool Win32Handle::operator==(const HANDLE other) const
	{
		return m_handle == other;
	}

	bool Win32Handle::operator==(const Win32Handle& other) const
	{
		return m_handle == other.m_handle;
	}

	HANDLE* Win32Handle::operator&()
	{
		return &m_handle;
	}

	HANDLE Win32Handle::DuplicateCurrentHandle() const
	{
		if (m_handle == nullptr)
			return nullptr;

		return DuplicatePassedHandle(m_handle, m_inheritable);
	}

	HANDLE Win32Handle::DuplicatePassedHandle(const HANDLE handle, const bool isInheritable) const
	{
		if (handle == nullptr)
			return nullptr;

		HANDLE duplicateHandle = nullptr;
		bool succeeded = DuplicateHandle(
			GetCurrentProcess(),
			handle,
			GetCurrentProcess(),
			&duplicateHandle,
			0,
			isInheritable,
			DUPLICATE_SAME_ACCESS
		);
		if (succeeded == false)
			throw std::runtime_error("Failed to duplicate handle.");

		return duplicateHandle;
	}

	bool Win32Handle::IsInheritable() const
	{
		return m_inheritable;
	}

	void Win32Handle::operator=(const bool inheritable)
	{
		m_inheritable = inheritable;
	}
}