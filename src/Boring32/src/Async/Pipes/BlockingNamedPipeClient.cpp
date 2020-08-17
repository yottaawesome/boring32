#include "pch.hpp"
#include <stdexcept>
#include "include/Error/Win32Exception.hpp"
#include "include/Async/Pipes/BlockingNamedPipeClient.hpp"

namespace Boring32::Async
{
	BlockingNamedPipeClient::~BlockingNamedPipeClient() { }

	BlockingNamedPipeClient::BlockingNamedPipeClient(const std::wstring& name)
	: NamedPipeClientBase(name)
	{ }

	BlockingNamedPipeClient::BlockingNamedPipeClient(const BlockingNamedPipeClient& other)
		: NamedPipeClientBase(other)
	{
		Copy(other);
	}

	void BlockingNamedPipeClient::operator=(const BlockingNamedPipeClient& other)
	{
		Copy(other);
	}

	BlockingNamedPipeClient::BlockingNamedPipeClient(BlockingNamedPipeClient&& other) noexcept
		: NamedPipeClientBase(other)
	{
		Move(other);
	}

	void BlockingNamedPipeClient::operator=(BlockingNamedPipeClient&& other) noexcept
	{
		Move(other);
	}

	void BlockingNamedPipeClient::Write(const std::wstring& msg)
	{
		if (m_handle == nullptr)
			throw std::runtime_error("No pipe to write to");

		DWORD bytesWritten = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
		bool successfulWrite = WriteFile(
			m_handle.GetHandle(),   // pipe handle 
			msg.c_str(),        // message 
			msg.size(),         // message length 
			&bytesWritten,      // bytes written 
			nullptr);           // not overlapped 

		if (successfulWrite == false)
			throw Error::Win32Exception("Failed to write to client pipe", GetLastError());
	}

	std::wstring BlockingNamedPipeClient::Read()
	{
		if (m_handle == nullptr)
			throw std::runtime_error("No pipe to read from");

		std::wstring dataBuffer;
		constexpr DWORD blockSize = 1024;
		dataBuffer.resize(1024);

		bool continueReading = true;
		DWORD totalBytesRead = 0;
		while (continueReading)
		{
			DWORD currentBytesRead = 0;
			// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-readfile
			bool successfulRead = ReadFile(
				m_handle.GetHandle(),    // pipe handle 
				&dataBuffer[0],    // buffer to receive reply 
				dataBuffer.size() * sizeof(TCHAR),  // size of buffer 
				&currentBytesRead,  // number of bytes read 
				nullptr);    // not overlapped
			totalBytesRead += currentBytesRead;
			
			const DWORD lastError = GetLastError();
			if (successfulRead == false && lastError != ERROR_MORE_DATA)
				throw Error::Win32Exception("Failed to read from pipe", GetLastError());
			if (lastError == ERROR_MORE_DATA)
				dataBuffer.resize(dataBuffer.size() + blockSize);
			continueReading = !successfulRead;
		}

		if (totalBytesRead > 0)
			dataBuffer.resize(totalBytesRead / sizeof(wchar_t));

		return dataBuffer;
	}
}