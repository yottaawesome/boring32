export module boring32:ipc_blockingnamedpipeserver;
import boring32.shared;
import :util;
import :raii;
import :error;
import :ipc_namedpipeserverbase;

export namespace Boring32::IPC
{
	struct BlockingNamedPipeServer : NamedPipeServerBase
	{
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
			Win32::PipeAccessDuplex,
			Win32::PipeTypeMessage           // message type pipe 
			| Win32::PipeReadModeMessage     // message-read mode
			| Win32::PipeWait
			| (isLocalPipe ? Win32::PipeRejectRemoteClients : Win32::PipeAcceptRemoteClients)
		)
		{
			m_openMode &= ~Win32::FileFlagOverlapped;
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
			m_openMode &= ~Win32::FileFlagOverlapped; // Negate overlapped flag
			InternalCreatePipe();
		}

		virtual void operator=(const BlockingNamedPipeServer& other)
		{
			Copy(other);
		}

		virtual void operator=(BlockingNamedPipeServer&& other) noexcept
		{
			Move(other);
		}

		virtual void Connect()
		{
			if (not m_pipe)
				throw Error::Boring32Error("No valid pipe handle to connect");

			if (not Win32::ConnectNamedPipe(m_pipe.GetHandle(), nullptr))
				throw Error::Boring32Error("Failed to connect named pipe");
		}

		virtual void Write(const std::wstring& msg)
		{
			InternalWrite(Util::StringToByteVector(msg));
		}

		virtual bool Write(const std::wstring& msg, std::nothrow_t) noexcept 
		try
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

		virtual bool ReadAsString(std::wstring& out, std::nothrow_t) noexcept 
		try
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
			if (not m_pipe)
				throw Error::Boring32Error("No pipe to write to");

			Win32::DWORD bytesWritten = 0;
			const bool success = Win32::WriteFile(
				m_pipe.GetHandle(),     // handle to pipe 
				&msg[0],                // buffer to write from 
				static_cast<Win32::DWORD>(msg.size()), // number of bytes to write 
				&bytesWritten,          // number of bytes written 
				nullptr                 // not overlapped I/O
			);
			if (not success)
				throw Error::Boring32Error("Failed to read pipe");
		}

		virtual std::vector<std::byte> InternalRead()
		{
			if (not m_pipe)
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
				if (not successfulRead and lastError != Win32::ErrorCodes::MoreData)
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
