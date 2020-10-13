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

		OverlappedIo oio;
		// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
		bool succeeded = WriteFile(
			m_handle.GetHandle(),					// pipe handle 
			msg.c_str(),							// message 
			(DWORD)(msg.size()*sizeof(wchar_t)),	// message length, in bytes
			nullptr,								// bytes written 
			oio.GetOverlapped());					// overlapped 
		oio.LastError(GetLastError());
		if (throwOnWin32Error && succeeded == false && oio.LastError() != ERROR_IO_PENDING)
			throw Error::Win32Error("OverlappedNamedPipeClient::Write(): WriteFile() failed", oio.LastError());

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

		OverlappedIo oio;
		oio.IoBuffer.resize(noOfCharacters);
		// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-readfile
		bool succeeded = ReadFile(
			m_handle.GetHandle(),							// pipe handle 
			&oio.IoBuffer[0],								// buffer to receive reply 
			(DWORD)(oio.IoBuffer.size()*sizeof(wchar_t)),	// size of buffer, in bytes 
			nullptr,										// number of bytes read 
			oio.GetOverlapped());							// overlapped
		oio.LastError(GetLastError());
		if (
			throwOnWin32Error 
			&& succeeded == false 
			&& oio.LastError() != ERROR_IO_PENDING
			&& oio.LastError() != ERROR_MORE_DATA
		)
		{
			throw Error::Win32Error("OverlappedNamedPipeClient::Read(): ReadFile() failed", oio.LastError());
		}

		return oio;
	}
}