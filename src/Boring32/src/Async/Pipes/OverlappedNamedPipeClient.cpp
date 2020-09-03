#include "pch.hpp"
#include <stdexcept>
#include "include/Error/Win32Error.hpp"
#include "include/Async/Pipes/OverlappedNamedPipeClient.hpp"

namespace Boring32::Async
{
	OverlappedNamedPipeClient::~OverlappedNamedPipeClient() { }

	OverlappedNamedPipeClient::OverlappedNamedPipeClient() { }

	OverlappedNamedPipeClient::OverlappedNamedPipeClient(const std::wstring& name)
		: NamedPipeClientBase(name, FILE_FLAG_OVERLAPPED)
	{ }

	OverlappedNamedPipeClient::OverlappedNamedPipeClient(const OverlappedNamedPipeClient& other)
		: NamedPipeClientBase(other)
	{
		Copy(other);
	}

	void OverlappedNamedPipeClient::operator=(const OverlappedNamedPipeClient& other)
	{
		Copy(other);
	}

	OverlappedNamedPipeClient::OverlappedNamedPipeClient(OverlappedNamedPipeClient&& other) noexcept
		: NamedPipeClientBase(std::move(other))
	{
		Move(other);
	}

	void OverlappedNamedPipeClient::operator=(OverlappedNamedPipeClient&& other) noexcept
	{
		Move(other);
	}

	OverlappedIo OverlappedNamedPipeClient::Write(const std::wstring& msg)
	{
		return InternalWrite(msg, true);
	}

	OverlappedIo OverlappedNamedPipeClient::Write(const std::wstring& msg, const std::nothrow_t)
	{
		return InternalWrite(msg, false);
	}

	OverlappedIo OverlappedNamedPipeClient::InternalWrite(const std::wstring& msg, const bool throwOnWin32Error)
	{
		if (m_handle == nullptr)
			throw std::runtime_error("No pipe to write to");

		OverlappedIo oio(m_handle);
		// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
		oio.CallReturnValue = WriteFile(
			m_handle.GetHandle(),   // pipe handle 
			msg.c_str(),        // message 
			msg.size(),         // message length 
			nullptr,      // bytes written 
			oio.GetOverlapped());           // not overlapped 
		oio.LastErrorValue = GetLastError();
		if (throwOnWin32Error && oio.CallReturnValue == false && oio.LastErrorValue != ERROR_IO_PENDING)
			throw Error::Win32Error("OverlappedNamedPipeClient::Write(): WriteFile() failed", oio.LastErrorValue);

		return oio;
	}

	OverlappedIo OverlappedNamedPipeClient::Read(const DWORD noOfCharacters)
	{
		return InternalRead(noOfCharacters, true);
	}

	OverlappedIo OverlappedNamedPipeClient::Read(const DWORD noOfCharacters, const std::nothrow_t)
	{
		return InternalRead(noOfCharacters, false);
	}

	OverlappedIo OverlappedNamedPipeClient::InternalRead(const DWORD noOfCharacters, const bool throwOnWin32Error)
	{
		if (m_handle == nullptr)
			throw std::runtime_error("No pipe to read from");

		OverlappedIo oio(m_handle);
		oio.IoBuffer.resize(noOfCharacters);

		oio.CallReturnValue = ReadFile(
			m_handle.GetHandle(),    // pipe handle 
			&oio.IoBuffer[0],    // buffer to receive reply 
			oio.IoBuffer.size() * sizeof(TCHAR),  // size of buffer 
			nullptr,  // number of bytes read 
			oio.GetOverlapped());    // overlapped
		oio.LastErrorValue = GetLastError();
		if (throwOnWin32Error && oio.CallReturnValue == false && oio.LastErrorValue != ERROR_IO_PENDING)
			throw Error::Win32Error("OverlappedNamedPipeClient::Read(): ReadFile() failed", oio.LastErrorValue);

		return oio;
	}
}