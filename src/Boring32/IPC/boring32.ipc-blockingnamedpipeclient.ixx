module;

#include <source_location>;

export module boring32.ipc:blockingnamedpipeclient;
import <vector>;
import <string>;
import <stdexcept>;
import <win32.hpp>;
import boring32.error;
import boring32.util;
import :namedpipeclientbase;

export namespace Boring32::IPC
{
	class BlockingNamedPipeClient final : public NamedPipeClientBase
	{
		public:
			~BlockingNamedPipeClient() = default;
			BlockingNamedPipeClient() = default;
			BlockingNamedPipeClient(const BlockingNamedPipeClient& other) = default;
			BlockingNamedPipeClient(BlockingNamedPipeClient&& other) noexcept = default;
			BlockingNamedPipeClient& operator=(const BlockingNamedPipeClient& other) = default;
			BlockingNamedPipeClient& operator=(BlockingNamedPipeClient&& other) noexcept = default;

		public:
			BlockingNamedPipeClient(const std::wstring& name)
				: NamedPipeClientBase(name, 0)
			{ }

		public:
			void Write(const std::wstring& msg)
			{
				InternalWrite(Util::StringToByteVector(msg));
			}

			bool Write(
				const std::wstring& msg, 
				const std::nothrow_t&
			) noexcept try
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

			bool Write(
				const std::vector<std::byte>& data, 
				const std::nothrow_t&
			) noexcept try
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

			bool ReadAsString(
				std::wstring& out, 
				const std::nothrow_t&
			) noexcept try
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
				if (!m_handle)
					throw Error::Boring32Error("No pipe to write to");

				DWORD bytesWritten = 0;
				// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
				const bool successfulWrite = WriteFile(
					m_handle.GetHandle(),   // pipe handle 
					&data[0],        // message 
					static_cast<DWORD>(data.size()),         // message length 
					&bytesWritten,      // bytes written 
					nullptr				// not overlapped 
				);
				if (!successfulWrite)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("Failed to write to client pipe", lastError);
				}
			}

			std::vector<std::byte> InternalRead()
			{
				if (!m_handle)
					throw Error::Boring32Error("No pipe to read from");

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
						static_cast<DWORD>(dataBuffer.size()),  // size of buffer 
						&currentBytesRead,  // number of bytes read 
						nullptr				// not overlapped
					);
					totalBytesRead += currentBytesRead;

					const DWORD lastError = GetLastError();
					if (successfulRead == false && lastError != ERROR_MORE_DATA)
					{
						const auto lastError = GetLastError();
						throw Error::Win32Error("Failed to read from pipe", lastError);
					}
					if (lastError == ERROR_MORE_DATA)
						dataBuffer.resize(dataBuffer.size() + blockSize);
					continueReading = !successfulRead;
				}

				if (totalBytesRead > 0)
					dataBuffer.resize(totalBytesRead);

				return dataBuffer;
			}
	};
}