#include "pch.hpp"
#include <stdexcept>
#include "include/Error/Win32Error.hpp"
#include "include/Async/Pipes/NamedPipeClientBase.hpp"

namespace Boring32::Async
{
	NamedPipeClientBase::~NamedPipeClientBase()
	{
		Close();
	}

	NamedPipeClientBase::NamedPipeClientBase()
		: m_pipeName(L""),
		m_fileAttributes(0)
	{ }

	NamedPipeClientBase::NamedPipeClientBase(const std::wstring& name, const DWORD fileAttributes)
	:	m_pipeName(name),
		m_fileAttributes(fileAttributes)
	{ }

	NamedPipeClientBase::NamedPipeClientBase(const NamedPipeClientBase& other)
	{
		Copy(other);
	}

	void NamedPipeClientBase::operator=(const NamedPipeClientBase& other)
	{
		Copy(other);
	}

	void NamedPipeClientBase::Copy(const NamedPipeClientBase& other)
	{
		m_handle = other.m_handle;
		m_pipeName = other.m_pipeName;
		m_fileAttributes = other.m_fileAttributes;
	}

	NamedPipeClientBase::NamedPipeClientBase(NamedPipeClientBase&& other) noexcept
	{
		Move(other);
	}

	void NamedPipeClientBase::operator=(NamedPipeClientBase&& other) noexcept
	{
		Move(other);
	}

	void NamedPipeClientBase::Move(NamedPipeClientBase& other) noexcept
	{
		m_handle = std::move(other.m_handle);
		m_pipeName = std::move(other.m_pipeName);
		m_fileAttributes = other.m_fileAttributes;
	}

	void NamedPipeClientBase::Connect(const DWORD timeout)
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew
		m_handle = CreateFileW(
			m_pipeName.c_str(),   // pipe name 
			GENERIC_READ | GENERIC_WRITE,// read and write access 
			0,              // no sharing 
			nullptr,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			m_fileAttributes, // attributes 
			nullptr);          // no template file 
		if (m_handle == INVALID_HANDLE_VALUE)
		{
			if (GetLastError() != ERROR_PIPE_BUSY || timeout == 0)
				throw Error::Win32Error("Failed to connect client pipe", GetLastError());
			if (WaitNamedPipeW(m_pipeName.c_str(), timeout) == false)
				throw Error::Win32Error("Failed to connect client pipe: timeout", GetLastError());
		}
	}

	void NamedPipeClientBase::SetMode(const DWORD pipeMode)
	{
		//PIPE_READMODE_MESSAGE or PIPE_READMODE_BYTE
		DWORD passedPipeMode = pipeMode;
		bool fSuccess = SetNamedPipeHandleState(
			m_handle.GetHandle(),    // pipe handle 
			&passedPipeMode,  // new pipe mode 
			nullptr,     // don't set maximum bytes 
			nullptr);    // don't set maximum time 
		if (fSuccess == false)
			throw Error::Win32Error("Failed to SetNamedPipeHandleState", GetLastError());
	}

	void NamedPipeClientBase::Close()
	{
		m_handle.Close();
	}

	DWORD NamedPipeClientBase::UnreadCharactersRemaining() const
	{
		if (m_handle == nullptr)
			throw std::runtime_error("No pipe to read from");
		DWORD bytesLeft = 0;
		bool succeeded = PeekNamedPipe(
			m_handle.GetHandle(),
			nullptr,
			0,
			nullptr,
			nullptr,
			&bytesLeft
		);
		if (succeeded == false)
			throw Error::Win32Error("PeekNamedPipe() failed", GetLastError());

		return bytesLeft / sizeof(wchar_t);
	}
}
