#include "pch.hpp"
#include <vector>
#include <stdexcept>
#include "include/Async/Pipes/BlockingNamedPipeClient.hpp"

import boring32.error.win32error;
import boring32.util;

namespace Boring32::Async
{
	BlockingNamedPipeClient::~BlockingNamedPipeClient() { }

	BlockingNamedPipeClient::BlockingNamedPipeClient() { }

	BlockingNamedPipeClient::BlockingNamedPipeClient(const std::wstring& name)
	: NamedPipeClientBase(name, 0)
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
		: NamedPipeClientBase(std::move(other))
	{
		Move(other);
	}

	void BlockingNamedPipeClient::operator=(BlockingNamedPipeClient&& other) noexcept
	{
		Move(other);
	}

	void BlockingNamedPipeClient::Write(const std::wstring& msg)
	{
		InternalWrite(Util::StringToByteVector(msg));
	}
	
	bool BlockingNamedPipeClient::Write(const std::wstring& msg, const std::nothrow_t)
	{
		try
		{
			InternalWrite(Util::StringToByteVector(msg));
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	void BlockingNamedPipeClient::InternalWrite(const std::vector<std::byte>& data)
	{
		if (m_handle == nullptr)
			throw std::runtime_error("No pipe to write to");

		DWORD bytesWritten = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
		bool successfulWrite = WriteFile(
			m_handle.GetHandle(),   // pipe handle 
			&data[0],        // message 
			(DWORD)data.size(),         // message length 
			&bytesWritten,      // bytes written 
			nullptr				// not overlapped 
		);           
		if (successfulWrite == false)
			throw Error::Win32Error("Failed to write to client pipe", GetLastError());
	}

	std::wstring BlockingNamedPipeClient::Read()
	{
		return Util::ByteVectorToString<std::wstring>(InternalRead());
	}

	bool BlockingNamedPipeClient::Read(std::wstring& out, const std::nothrow_t)
	{
		try
		{
			out = Util::ByteVectorToString<std::wstring>(InternalRead());
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	std::vector<std::byte> BlockingNamedPipeClient::InternalRead()
	{
		if (m_handle == nullptr)
			throw std::runtime_error("No pipe to read from");

		constexpr DWORD blockSize = 1024;
		std::vector<std::byte> dataBuffer(blockSize);

		bool continueReading = true;
		DWORD totalBytesRead = 0;
		while (continueReading)
		{
			DWORD currentBytesRead = 0;
			// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-readfile
			bool successfulRead = ReadFile(
				m_handle.GetHandle(),    // pipe handle 
				&dataBuffer[0],    // buffer to receive reply 
				(DWORD)dataBuffer.size(),  // size of buffer 
				&currentBytesRead,  // number of bytes read 
				nullptr				// not overlapped
			);    
			totalBytesRead += currentBytesRead;
			
			const DWORD lastError = GetLastError();
			if (successfulRead == false && lastError != ERROR_MORE_DATA)
				throw Error::Win32Error("Failed to read from pipe", GetLastError());
			if (lastError == ERROR_MORE_DATA)
				dataBuffer.resize(dataBuffer.size() + blockSize);
			continueReading = !successfulRead;
		}

		if (totalBytesRead > 0)
			dataBuffer.resize(totalBytesRead / sizeof(wchar_t));

		return dataBuffer;
	}
}