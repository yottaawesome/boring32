export module boring32:ipc.blockingnamedpipeclient;
import :win32;
import :error;
import :util;
import :ipc.namedpipeclientbase;

export namespace Boring32::IPC
{
	struct BlockingNamedPipeClient final : NamedPipeClientBase
	{
		BlockingNamedPipeClient() = default;

		BlockingNamedPipeClient(const std::wstring& name)
			: NamedPipeClientBase(name, 0)
		{ }

		void Write(const std::wstring& msg)
		{
			InternalWrite(Util::StringToByteVector(msg));
		}

		bool Write(const std::wstring& msg, std::nothrow_t) noexcept 
		try
		{
			InternalWrite(Util::StringToByteVector(msg));
			return true;
		}
		catch (...)
		{
			return false;
		}

		void Write(const std::vector<std::byte>& data)
		{
			InternalWrite(data);
		}

		bool Write(const std::vector<std::byte>& data, std::nothrow_t) noexcept
		try
		{
			InternalWrite(data);
			return true;
		}
		catch (...)
		{
			return false;
		}

		std::wstring ReadAsString()
		{
			return Util::ByteVectorToString<std::wstring>(InternalRead());
		}

		bool ReadAsString(std::wstring& out, std::nothrow_t) noexcept 
		try
		{
			out = Util::ByteVectorToString<std::wstring>(InternalRead());
			return true;
		}
		catch (...)
		{
			return false;
		}

		private:
		void InternalWrite(const std::vector<std::byte>& data)
		{
			if (not m_handle)
				throw Error::Boring32Error{ "No pipe to write to" };

			Win32::DWORD bytesWritten = 0;
			// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
			const bool successfulWrite = Win32::WriteFile(
				m_handle.GetHandle(),   // pipe handle 
				&data[0],        // message 
				static_cast<Win32::DWORD>(data.size()),         // message length 
				&bytesWritten,      // bytes written 
				nullptr				// not overlapped 
			);
			if (not successfulWrite)
				throw Error::Win32Error{Win32::GetLastError(), "Failed to write to client pipe"};
		}

		auto InternalRead() -> std::vector<std::byte>
		{
			if (not m_handle)
				throw Error::Boring32Error{ "No pipe to read from" };

			constexpr auto blockSize = Win32::DWORD{ 1024 };
			auto dataBuffer = std::vector<std::byte>(blockSize);

			auto continueReading = true;
			auto totalBytesRead = Win32::DWORD{};
			while (continueReading)
			{
				auto currentBytesRead = Win32::DWORD{};
				// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-readfile
				auto successfulRead = Win32::ReadFile(
					m_handle.GetHandle(),    // pipe handle 
					&dataBuffer[0],    // buffer to receive reply 
					static_cast<Win32::DWORD>(dataBuffer.size()),  // size of buffer 
					&currentBytesRead,  // number of bytes read 
					nullptr				// not overlapped
				);
				totalBytesRead += currentBytesRead;

				auto lastError = Win32::GetLastError();
				if (not successfulRead and lastError != Win32::ErrorCodes::MoreData)
					throw Error::Win32Error{lastError, "Failed to read from pipe"};
				if (lastError == Win32::ErrorCodes::MoreData)
					dataBuffer.resize(dataBuffer.size() + blockSize);
				continueReading = !successfulRead;
			}

			if (totalBytesRead > 0)
				dataBuffer.resize(totalBytesRead);

			return dataBuffer;
		}
	};
}