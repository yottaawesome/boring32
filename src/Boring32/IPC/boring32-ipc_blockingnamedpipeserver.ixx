export module boring32:ipc_blockingnamedpipeserver;
import boring32.shared;
import :util;
import :raii;
import :error;
import :ipc_namedpipeserverbase;

export namespace Boring32::IPC
{
	class BlockingNamedPipeServer : public NamedPipeServerBase
	{
		public:
			virtual ~BlockingNamedPipeServer()
			{
				Close();
			}

			BlockingNamedPipeServer(const BlockingNamedPipeServer& other)
				: NamedPipeServerBase(other)
			{
				InternalCreatePipe();
			}

			BlockingNamedPipeServer(BlockingNamedPipeServer&& other) noexcept
				: NamedPipeServerBase(std::move(other))
			{
				InternalCreatePipe();
			}

			BlockingNamedPipeServer(
				const std::wstring& pipeName, 
				const Win32::DWORD size,
				const Win32::DWORD maxInstances, // PIPE_UNLIMITED_INSTANCES
				const std::wstring& sid,
				const bool isInheritable,
				const bool isLocalPipe
			) : NamedPipeServerBase(
				pipeName,
				size,
				maxInstances,
				sid,
				isInheritable,
				Win32::_PIPE_ACCESS_DUPLEX,
				Win32::_PIPE_TYPE_MESSAGE           // message type pipe 
				| Win32::_PIPE_READMODE_MESSAGE     // message-read mode
				| Win32::_PIPE_WAIT
				| (isLocalPipe
					? Win32::_PIPE_REJECT_REMOTE_CLIENTS
					: Win32::_PIPE_ACCEPT_REMOTE_CLIENTS)
			)
			{
				m_openMode &= ~Win32::_FILE_FLAG_OVERLAPPED;
				InternalCreatePipe();
			}

			BlockingNamedPipeServer(
				const std::wstring& pipeName,
				const Win32::DWORD size,
				const Win32::DWORD maxInstances, // PIPE_UNLIMITED_INSTANCES
				const std::wstring& sid,
				const bool isInheritable,
				const Win32::DWORD openMode,
				const Win32::DWORD pipeMode
			) : NamedPipeServerBase(
				pipeName,
				size,
				maxInstances,
				sid,
				isInheritable,
				openMode,
				pipeMode
			)
			{
				m_openMode &= ~Win32::_FILE_FLAG_OVERLAPPED; // Negate overlapped flag
				InternalCreatePipe();
			}

		public:
			virtual void operator=(const BlockingNamedPipeServer& other)
			{
				Copy(other);
			}

			virtual void operator=(BlockingNamedPipeServer&& other) noexcept
			{
				Move(other);
			}

		public:
			virtual void Connect()
			{
				if (!m_pipe)
					throw Error::Boring32Error("No valid pipe handle to connect");

				if (!Win32::ConnectNamedPipe(m_pipe.GetHandle(), nullptr))
					throw Error::Boring32Error("Failed to connect named pipe");
			}

			virtual void Write(const std::wstring& msg)
			{
				InternalWrite(Util::StringToByteVector(msg));
			}

			virtual bool Write(
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

			virtual std::wstring ReadAsString()
			{
				return Util::ByteVectorToString<std::wstring>(InternalRead());
			}

			virtual bool ReadAsString(
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

		protected:
			virtual void InternalWrite(const std::vector<std::byte>& msg)
			{
				if (!m_pipe)
					throw Error::Boring32Error("No pipe to write to");

				Win32::DWORD bytesWritten = 0;
				const bool success = Win32::WriteFile(
					m_pipe.GetHandle(),     // handle to pipe 
					&msg[0],                // buffer to write from 
					static_cast<Win32::DWORD>(msg.size()), // number of bytes to write 
					&bytesWritten,          // number of bytes written 
					nullptr                 // not overlapped I/O
				);
				if (!success)
					throw Error::Boring32Error("Failed to read pipe");
			}

			virtual std::vector<std::byte> InternalRead()
			{
				if (!m_pipe)
					throw Error::Boring32Error("No pipe to read from");

				constexpr Win32::DWORD blockSize = 1024;
				std::vector<std::byte> dataBuffer(blockSize);

				bool continueReading = true;
				Win32::DWORD totalBytesRead = 0;
				while (continueReading)
				{
					Win32::DWORD currentBytesRead = 0;
					// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-readfile
					const bool successfulRead = Win32::ReadFile(
						m_pipe.GetHandle(),    // pipe handle 
						&dataBuffer[0],    // buffer to receive reply 
						static_cast<Win32::DWORD>(dataBuffer.size()),  // size of buffer 
						&currentBytesRead,  // number of bytes read 
						nullptr // not overlapped
					);
					totalBytesRead += currentBytesRead;

					const DWORD lastError = Win32::GetLastError();
					if (!successfulRead && lastError != Win32::ErrorCodes::MoreData)
						throw Error::Boring32Error("Failed to read from pipe");
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
