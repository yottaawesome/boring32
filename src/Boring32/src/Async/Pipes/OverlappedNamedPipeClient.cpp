#include "pch.hpp"
#include <stdexcept>
#include "include/Async/Pipes/OverlappedNamedPipeClient.hpp"

namespace Boring32::Async
{
	OverlappedNamedPipeClient::~OverlappedNamedPipeClient() { }

	OverlappedNamedPipeClient::OverlappedNamedPipeClient(const std::wstring& name)
		: NamedPipeClientBase(name)
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
		: NamedPipeClientBase(other)
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
		DWORD bytesWritten = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
		oio.CallReturnValue = WriteFile(
			m_handle.GetHandle(),   // pipe handle 
			msg.c_str(),        // message 
			msg.size(),         // message length 
			&bytesWritten,      // bytes written 
			&oio.IoOverlapped);           // not overlapped 
		oio.LastErrorValue = GetLastError();
		if (oio.CallReturnValue == false && oio.LastErrorValue != ERROR_IO_PENDING)
			throw std::runtime_error("Failed to write to pipe");

		return oio;
	}

	OverlappedIo OverlappedNamedPipeClient::Read(std::wstring& dataBuffer)
	{
		if (m_handle == nullptr)
			throw std::runtime_error("No pipe to read from");

		constexpr DWORD blockSize = 1024;
		if (dataBuffer.size() < blockSize)
			dataBuffer.resize(blockSize);

		OverlappedIo oio;
		DWORD totalBytesRead = 0;
		bool continueReading = true;
		while (continueReading)
		{
			DWORD currentBytesRead = 0;
			oio.CallReturnValue = ReadFile(
				m_handle.GetHandle(),    // pipe handle 
				&dataBuffer[0],    // buffer to receive reply 
				dataBuffer.size() * sizeof(TCHAR),  // size of buffer 
				&currentBytesRead,  // number of bytes read 
				&oio.IoOverlapped);    // overlapped
			totalBytesRead += currentBytesRead;
			oio.LastErrorValue = GetLastError();
			if (oio.LastErrorValue == ERROR_MORE_DATA)
				dataBuffer.resize(dataBuffer.size() + blockSize);
			else if (oio.LastErrorValue != ERROR_IO_PENDING)
				throw std::runtime_error("Failed to read from pipe");
		}

		if (totalBytesRead > 0)
			dataBuffer.resize(totalBytesRead / sizeof(wchar_t));

		return oio;
	}
}