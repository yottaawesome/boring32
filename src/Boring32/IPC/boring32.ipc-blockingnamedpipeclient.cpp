module boring32.ipc:blockingnamedpipeclient;
import boring32.error;
import boring32.util;
import <stdexcept>;
import <win32.hpp>;

namespace Boring32::IPC
{
	BlockingNamedPipeClient::BlockingNamedPipeClient(const std::wstring& name)
	: NamedPipeClientBase(name, 0)
	{ }

	void BlockingNamedPipeClient::Write(const std::wstring& msg)
	{
		InternalWrite(Util::StringToByteVector(msg));
	}
	
	bool BlockingNamedPipeClient::Write(
		const std::wstring& msg, 
		const std::nothrow_t
	) noexcept try
	{
		InternalWrite(Util::StringToByteVector(msg));
		return true;
	}
	catch (...)
	{
		return false;
	}

	void BlockingNamedPipeClient::Write(const std::vector<std::byte>& data)
	{
		InternalWrite(data);
	}

	bool BlockingNamedPipeClient::Write(
		const std::vector<std::byte>& data, 
		const std::nothrow_t
	) noexcept try
	{
		InternalWrite(data);
		return true;
	}
	catch (...)
	{
		return false;
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

	std::wstring BlockingNamedPipeClient::ReadAsString()
	{
		return Util::ByteVectorToString<std::wstring>(InternalRead());
	}

	bool BlockingNamedPipeClient::ReadAsString(
		std::wstring& out, 
		const std::nothrow_t
	) noexcept try
	{
		out = Util::ByteVectorToString<std::wstring>(InternalRead());
		return true;
	}
	catch (...)
	{
		return false;
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
			dataBuffer.resize(totalBytesRead);

		return dataBuffer;
	}
}