module;

#include <string>
#include <iostream>
#include <stdexcept>
#include <Windows.h>

module boring32.ipc.namedpipeclientbase;
import boring32.error.win32error;

namespace Boring32::IPC
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
		Close();
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
		Close();
		m_handle = std::move(other.m_handle);
		m_pipeName = std::move(other.m_pipeName);
		m_fileAttributes = other.m_fileAttributes;
	}

	void NamedPipeClientBase::Connect(const DWORD timeout)
	{
		static std::wstring pipePrefix = LR"(\\.\pipe\)";
		if (m_pipeName.starts_with(pipePrefix) == false)
			m_pipeName = pipePrefix + m_pipeName;

		// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew
		m_handle = CreateFileW(
			m_pipeName.c_str(), // pipe name 
			GENERIC_READ | GENERIC_WRITE,// read and write access 
			0,					// no sharing 
			nullptr,			// default security attributes
			OPEN_EXISTING,		// opens existing pipe 
			m_fileAttributes,	// attributes 
			nullptr				// no template file
		);           
		if (m_handle == INVALID_HANDLE_VALUE)
		{
			if (GetLastError() != ERROR_PIPE_BUSY || timeout == 0)
				throw Error::Win32Error(__FUNCSIG__": failed to connect client pipe", GetLastError());
			if (WaitNamedPipeW(m_pipeName.c_str(), timeout) == false)
				throw Error::Win32Error(__FUNCSIG__": timed out trying to connect client pipe", GetLastError());
		}
	}

	bool NamedPipeClientBase::Connect(const DWORD timeout, std::nothrow_t)
	{
		try
		{
			Connect(timeout);
			return true;
		}
		catch (const std::exception& ex)
		{
			std::wcerr
				<< __FUNCSIG__ << L" failed: "
				<< ex.what()
				<< std::endl;
			return false;
		}
	}

	void NamedPipeClientBase::SetMode(const DWORD pipeMode)
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/namedpipeapi/nf-namedpipeapi-setnamedpipehandlestate?redirectedfrom=MSDN
		//PIPE_READMODE_MESSAGE or PIPE_READMODE_BYTE
		DWORD passedPipeMode = pipeMode;
		const bool succeeded = SetNamedPipeHandleState(
			m_handle.GetHandle(),    // pipe handle 
			&passedPipeMode,  // new pipe mode 
			nullptr,     // don't set maximum bytes 
			nullptr);    // don't set maximum time 
		if (!succeeded)
			throw Error::Win32Error(__FUNCSIG__ ": SetNamedPipeHandleState() failed", GetLastError());
	}

	void NamedPipeClientBase::Close()
	{
		m_handle.Close();
	}

	DWORD NamedPipeClientBase::UnreadCharactersRemaining() const
	{
		if (!m_handle)
			throw std::runtime_error(__FUNCSIG__ ": no pipe to read from");
		DWORD bytesLeft = 0;
		const bool succeeded = PeekNamedPipe(
			m_handle.GetHandle(),
			nullptr,
			0,
			nullptr,
			nullptr,
			&bytesLeft
		);
		if (!succeeded)
			throw Error::Win32Error(__FUNCSIG__ ": PeekNamedPipe() failed", GetLastError());

		return bytesLeft / sizeof(wchar_t);
	}

	void NamedPipeClientBase::Flush()
	{
		if (!m_handle)
			throw std::runtime_error("No pipe to flush");
		if (FlushFileBuffers(m_handle.GetHandle()) == false)
			throw Error::Win32Error(__FUNCSIG__ ": FlushFileBuffers() failed", GetLastError());
	}

	void NamedPipeClientBase::CancelCurrentThreadIo()
	{
		if (!m_handle)
			throw std::runtime_error(__FUNCSIG__": pipe is nullptr");
		if (!CancelIo(m_handle.GetHandle()))
			throw Error::Win32Error(__FUNCSIG__": CancelIo failed", GetLastError());
	}

	bool NamedPipeClientBase::CancelCurrentThreadIo(std::nothrow_t) noexcept
	{
		try
		{
			CancelCurrentThreadIo();
			return true;
		}
		catch (const std::exception& ex)
		{
			std::wcerr << __FUNCSIG__ << L" failed: " << ex.what() << std::endl;
			return false;
		}
	}

	void NamedPipeClientBase::CancelCurrentProcessIo(OVERLAPPED* overlapped)
	{
		if (m_handle == nullptr)
			throw std::runtime_error(__FUNCSIG__": pipe is nullptr");
		if (CancelIoEx(m_handle.GetHandle(), overlapped) == false)
			throw Error::Win32Error(__FUNCSIG__": CancelIo() failed", GetLastError());
	}

	bool NamedPipeClientBase::CancelCurrentProcessIo(OVERLAPPED* overlapped, std::nothrow_t) noexcept
	{
		try
		{
			CancelCurrentProcessIo(overlapped);
			return true;
		}
		catch (const std::exception& ex)
		{
			std::wcerr << __FUNCSIG__ << L" failed: " << ex.what() << std::endl;
			return false;
		}
	}
}
