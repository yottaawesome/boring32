#include "pch.hpp"
#include <stdexcept>
#include "include/Error/Win32Error.hpp"
#include "include/Strings/Strings.hpp"
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
		return InternalWrite(msg);
	}

	bool OverlappedNamedPipeClient::Write(const std::wstring& msg, OverlappedIo& op) noexcept
	{
		try
		{
			op = InternalWrite(msg);
			return true;
		}
		catch (const std::exception& ex)
		{
			std::wcerr 
				<< L"OverlappedNamedPipeClient::Write(): " 
				<< ex.what() 
				<< std::endl;
			return false;
		}
	}

	OverlappedIo OverlappedNamedPipeClient::InternalWrite(const std::wstring& msg)
	{
		if (m_handle == nullptr)
			throw std::runtime_error("OverlappedNamedPipeClient::InternalWrite(): No pipe to write to");

		OverlappedIo oio;
		// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
		bool succeeded = WriteFile(
			m_handle.GetHandle(),					// pipe handle 
			msg.c_str(),							// message 
			(DWORD)(msg.size()*sizeof(wchar_t)),	// message length, in bytes
			nullptr,								// bytes written 
			oio.GetOverlapped());					// overlapped 
		oio.LastError(GetLastError());
		if (succeeded == false && oio.LastError() != ERROR_IO_PENDING)
			throw Error::Win32Error("OverlappedNamedPipeClient::InternalWrite(): WriteFile() failed", oio.LastError());

		return oio;
	}

	OverlappedIo OverlappedNamedPipeClient::Read(const DWORD noOfCharacters)
	{
		return InternalRead(noOfCharacters);
	}

	bool OverlappedNamedPipeClient::Read(const DWORD noOfCharacters, OverlappedIo& op) noexcept
	{
		try
		{
			op = InternalRead(noOfCharacters);
			return true;
		}
		catch (const std::exception& ex)
		{
			std::wcerr
				<< L"OverlappedNamedPipeClient::Read(): "
				<< ex.what()
				<< std::endl;
			return false;
		}
	}

	OverlappedIo OverlappedNamedPipeClient::InternalRead(const DWORD noOfCharacters)
	{
		if (m_handle == nullptr)
			throw std::runtime_error("OverlappedNamedPipeClient::InternalRead(): No pipe to read from");

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
			succeeded == false 
			&& oio.LastError() != ERROR_IO_PENDING
			&& oio.LastError() != ERROR_MORE_DATA
		)
		{
			throw Error::Win32Error("OverlappedNamedPipeClient::InternalRead(): ReadFile() failed", oio.LastError());
		}

		return oio;
	}
}