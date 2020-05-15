#include "pch.hpp"
#include <stdexcept>
#include <Windows.h>
#include "include/Win32Utils.hpp"

namespace Win32Utils::WinHttp
{
	WinHttpHandle::WinHttpHandle()
		: m_handle(nullptr)
	{ }

	WinHttpHandle::WinHttpHandle(HINTERNET handle)
		: m_handle(handle)
	{ }

	WinHttpHandle::WinHttpHandle(WinHttpHandle&& other) noexcept
	{
		m_handle = other.m_handle;
		other.m_handle = nullptr;
	}

	void WinHttpHandle::operator=(WinHttpHandle&& other) noexcept
	{
		Close();
		m_handle = other.m_handle;
		other.m_handle = nullptr;
	}

	void WinHttpHandle::operator=(const HINTERNET& handle)
	{
		Close();
		m_handle = handle;
	}

	HINTERNET WinHttpHandle::Get()
	{
		return m_handle;
	}

	bool WinHttpHandle::operator==(const HINTERNET other)
	{
		return m_handle == other;
	}

	WinHttpHandle::~WinHttpHandle()
	{
		Close();
	}

	void WinHttpHandle::Close()
	{
		if (m_handle != nullptr)
		{
			WinHttpCloseHandle(m_handle);
			m_handle = nullptr;
		}
	}
}