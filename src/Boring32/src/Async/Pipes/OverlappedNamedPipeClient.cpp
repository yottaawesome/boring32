#include "pch.hpp"
#include <stdexcept>
#include "include/Error/Win32Exception.hpp"
#include "include/Async/Pipes/OverlappedNamedPipeClient.hpp"

namespace Boring32::Async
{
	OverlappedNamedPipeClient::~OverlappedNamedPipeClient() { }

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
		if (m_handle == nullptr)
			throw std::runtime_error("No pipe to write to");

		OverlappedIo oio;
		oio.IoHandle = m_handle;
		DWORD bytesWritten = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
		oio.CallReturnValue = WriteFile(
			m_handle.GetHandle(),   // pipe handle 
			msg.c_str(),        // message 
			msg.size(),         // message length 
			&bytesWritten,      // bytes written 
			oio.GetOverlapped());           // not overlapped 
		oio.LastErrorValue = GetLastError();
		if (oio.CallReturnValue == false && oio.LastErrorValue != ERROR_IO_PENDING)
			throw Error::Win32Exception("Failed to write to pipe", oio.LastErrorValue);

		return oio;
	}

	OverlappedIo OverlappedNamedPipeClient::Read()
	{
		if (m_handle == nullptr)
			throw std::runtime_error("No pipe to read from");

		constexpr DWORD blockSize = 1024;
		OverlappedIo oio;
		if (oio.IoBuffer.size() < blockSize)
			oio.IoBuffer.resize(blockSize);

		oio.IoHandle = m_handle;
		DWORD totalBytesRead = 0;
		bool continueReading = true;
		while (continueReading)
		{
			DWORD currentBytesRead = 0;
			oio.CallReturnValue = ReadFile(
				m_handle.GetHandle(),    // pipe handle 
				&oio.IoBuffer[0],    // buffer to receive reply 
				oio.IoBuffer.size() * sizeof(TCHAR),  // size of buffer 
				&currentBytesRead,  // number of bytes read 
				oio.GetOverlapped());    // overlapped
			totalBytesRead += currentBytesRead;
			oio.LastErrorValue = GetLastError();
			if (oio.CallReturnValue == false)
			{
				if (oio.LastErrorValue == ERROR_MORE_DATA)
					oio.IoBuffer.resize(oio.IoBuffer.size() + blockSize);
				else if (oio.LastErrorValue == ERROR_IO_PENDING)
					continueReading = false;
				else
					throw Error::Win32Exception("Failed to read from pipe", oio.LastErrorValue);
			}
			else
			{
				continueReading = false;
			}
		}

		if (totalBytesRead > 0)
			oio.IoBuffer.resize(totalBytesRead / sizeof(wchar_t));

		return oio;
	}
}