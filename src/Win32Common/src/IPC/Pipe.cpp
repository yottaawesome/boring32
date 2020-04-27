#include "pch.hpp"
#include <stdexcept>
#include "include/Win32Utils.hpp"

namespace Win32Utils::IPC
{
	Pipe::Pipe(const bool inheritable, const DWORD size)
	:	m_readHandle(nullptr),
		m_writeHandle(nullptr),
		m_inheritable(inheritable),
		m_size(size)
	{
		SECURITY_ATTRIBUTES lp{ 0 };
		lp.nLength = sizeof(lp);
		lp.bInheritHandle = m_inheritable;
		bool succeeded = CreatePipe(&m_readHandle, &m_writeHandle, &lp, size);
		if (succeeded == false)
			throw std::runtime_error("Failed to create anonymous pipe");
	}

	Pipe::Pipe(const bool inheritable, const DWORD size, const HANDLE m_readHandle, const HANDLE m_writeHandle)
	:	m_readHandle(m_readHandle),
		m_writeHandle(m_writeHandle),
		m_inheritable(inheritable),
		m_size(size)
	{ }

	Pipe::~Pipe()
	{
		Cleanup();
	}

	void Pipe::Cleanup()
	{
		if (m_readHandle)
		{
			CloseHandle(m_readHandle);
			m_readHandle = nullptr;
		}
		if (m_writeHandle)
		{
			CloseHandle(m_writeHandle);
			m_writeHandle = nullptr;
		}
	}

	void Pipe::Write(const std::wstring& msg)
	{
		DWORD bytesWritten;
		bool bSuccess = WriteFile(
			m_writeHandle, 
			msg.data(), 
			msg.size()*sizeof(wchar_t), 
			&bytesWritten, 
			nullptr
		);
		if (bSuccess == false)
			throw std::runtime_error("Write operation failed");
	}

	std::wstring Pipe::ReadFromPipe()
	{
		std::wstring msg;
		DWORD bytesRead;
		msg.resize(m_size);
		bool bSuccess = ReadFile(m_readHandle, &msg[0], msg.size()*sizeof(wchar_t), &bytesRead, nullptr);
		if (bSuccess == false)
			throw std::runtime_error("Write operation failed");
		msg.erase(std::find(msg.begin(), msg.end(), '\0'), msg.end());
		
		return msg;
	}

	void Pipe::CloseRead()
	{
		if (m_readHandle)
		{
			CloseHandle(m_readHandle);
			m_readHandle = nullptr;
		}
	}
	
	void Pipe::CloseWrite()
	{
		if (m_writeHandle)
		{
			CloseHandle(m_writeHandle);
			m_writeHandle = nullptr;
		}
	}

	HANDLE Pipe::GetRead()
	{
		return m_readHandle;
	}
	
	HANDLE Pipe::GetWrite()
	{
		return m_writeHandle;
	}
}