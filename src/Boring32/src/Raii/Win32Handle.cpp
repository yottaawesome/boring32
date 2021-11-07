#include "pch.hpp"
#include <stdexcept>
#include <format>
#include "include/Error/Error.hpp"
#include "include/Raii/Win32Handle.hpp"
#include "include/Util/Util.hpp"

namespace Boring32::Raii
{
	void CloseHandleAndFreeMemory(HANDLE* pHandle)
	{
		std::wcout << L"Deleting handle\n";
		if (!pHandle)
			return;
		if (*pHandle && !CloseHandle(*pHandle))
			std::wcerr << "Failed to close handle\n";
		delete pHandle;
		std::wcout << L"Deleted handle\n";
	}

	std::shared_ptr<HANDLE> CreateClosableHandle(HANDLE handle)
	{
		return { new void* (handle), CloseHandleAndFreeMemory };
	}

	Win32Handle::~Win32Handle()
	{
		Close();
	}

	void Win32Handle::Close() noexcept
	{
		m_handle = nullptr;
	}

	Win32Handle::Win32Handle()
	:	m_handle(CreateClosableHandle(nullptr))
	{ }
	
	Win32Handle::Win32Handle(const Win32Handle& otherHandle)
	:	m_handle(CreateClosableHandle(nullptr))
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
		m_handle = other.m_handle;
			/*Win32Handle::DuplicatePassedHandle(
				other.GetHandle(), 
				other.IsInheritable()
			);*/
	}

	Win32Handle::Win32Handle(Win32Handle&& other) noexcept
	:	m_handle(CreateClosableHandle(nullptr))
	{
		Move(other);
	}

	Win32Handle& Win32Handle::operator=(Win32Handle&& other) noexcept
	{
		Move(other);
		return *this;
	}

	void Win32Handle::Move(Win32Handle& other) noexcept
	{
		Close();
		m_handle = std::move(other.m_handle);
		other.m_handle = nullptr;
	}

	Win32Handle::Win32Handle(const HANDLE handle)
	:	m_handle(CreateClosableHandle(handle))
	{ }

	Win32Handle& Win32Handle::operator=(const HANDLE other)
	{
		Close();
		m_handle = CreateClosableHandle(other);
		return *this;
	}

	bool Win32Handle::operator==(const HANDLE other) const
	{
		if (other == nullptr || *m_handle == INVALID_HANDLE_VALUE)
			return m_handle == nullptr || *m_handle == INVALID_HANDLE_VALUE;
		return *m_handle == other;
	}

	bool Win32Handle::operator==(const Win32Handle& other) const
	{
		// https://devblogs.microsoft.com/oldnewthing/20040302-00/?p=40443
		if (other.m_handle == nullptr || *other.m_handle == INVALID_HANDLE_VALUE)
			return m_handle == nullptr || *m_handle == INVALID_HANDLE_VALUE;
		return m_handle == other.m_handle;
	}

	HANDLE* Win32Handle::operator&()
	{
		if (m_handle == nullptr)
			m_handle = CreateClosableHandle(nullptr);
		return m_handle.get();
	}

	HANDLE Win32Handle::operator*() const noexcept
	{
		if (m_handle == nullptr)
			return nullptr;

		return *m_handle;
	}

	Win32Handle::operator bool() const noexcept
	{
		return IsValidValue();
	}

	bool Win32Handle::IsValidValue() const noexcept
	{
		return m_handle && *m_handle && *m_handle != INVALID_HANDLE_VALUE;
	}

	HANDLE Win32Handle::GetHandle() const noexcept
	{
		if (!m_handle)
			return nullptr;
		return *m_handle;
	}

	HANDLE Win32Handle::DuplicateCurrentHandle() const
	{
		if (!m_handle || *m_handle == nullptr)
			return nullptr;
		return Win32Handle::DuplicatePassedHandle(*m_handle, IsInheritable());
	}

	bool Win32Handle::IsInheritable() const
	{
		if (!m_handle || *m_handle == nullptr)
			return false;
		return Win32Handle::HandleIsInheritable(*m_handle);
	}

	void Win32Handle::SetInheritability(const bool isInheritable)
	{
		if (!IsValidValue())
			throw std::runtime_error(__FUNCSIG__": handle is null or invalid.");
		if (SetHandleInformation(*m_handle, HANDLE_FLAG_INHERIT, isInheritable) == false)
			throw Error::Win32Error(__FUNCSIG__": SetHandleInformation() failed", GetLastError());
	}

	HANDLE Win32Handle::Detach() noexcept
	{
		if (!m_handle || !*m_handle)
			return nullptr;
		HANDLE temp = *m_handle;
		*m_handle = nullptr;
		return temp;
	}

	bool Win32Handle::HandleIsInheritable(const HANDLE handle)
	{
		if (handle == nullptr || handle == INVALID_HANDLE_VALUE)
			return false;

		DWORD flags = 0;
		if (GetHandleInformation(handle, &flags) == false)
			throw Error::Win32Error(__FUNCSIG__": GetHandleInformation() failed", GetLastError());
		return flags & HANDLE_FLAG_INHERIT;
	}

	HANDLE Win32Handle::DuplicatePassedHandle(const HANDLE handle, const bool isInheritable)
	{
		if (handle == nullptr)
			return nullptr;
		if (handle == INVALID_HANDLE_VALUE)
			return INVALID_HANDLE_VALUE;

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
			throw Error::Win32Error(__FUNCSIG__": DuplicateHandle() failed", GetLastError());

		return duplicateHandle;
	}
}