export module boring32:ipc_overlappednamedpipeserver;
import boring32.shared;
import :raii;
import :error;
import :async;
import :ipc_namedpipeserverbase;

export namespace Boring32::IPC
{
	struct OverlappedNamedPipeServer final : NamedPipeServerBase
	{
		~OverlappedNamedPipeServer()
		{
			Close();
		}

		OverlappedNamedPipeServer() = default;

		OverlappedNamedPipeServer(const OverlappedNamedPipeServer& other)
		:   NamedPipeServerBase(other)
		{
			InternalCreatePipe();
		}

		OverlappedNamedPipeServer(OverlappedNamedPipeServer&& other) noexcept
			: NamedPipeServerBase(std::move(other))
		{
			InternalCreatePipe();
		}

		OverlappedNamedPipeServer(
			const std::wstring& pipeName, 
			Win32::DWORD size,
			Win32::DWORD maxInstances, // PIPE_UNLIMITED_INSTANCES
			const std::wstring& sid,
			bool isInheritable,
			bool isLocalPipe
		) : NamedPipeServerBase(
			pipeName,
			size,
			maxInstances,
			sid,
			isInheritable,
			Win32::PipeAccessDuplex | Win32::FileFlagOverlapped,
			Win32::PipeTypeMessage
			| Win32::PipeReadModeMessage 
			| Win32::PipeWait
			| (isLocalPipe ? Win32::PipeRejectRemoteClients : Win32::PipeAcceptRemoteClients)
		)
		{
			InternalCreatePipe();
		}

		OverlappedNamedPipeServer(
			const std::wstring& pipeName,
			Win32::DWORD size,
			Win32::DWORD maxInstances, // PIPE_UNLIMITED_INSTANCES
			const std::wstring& sid,
			bool isInheritable,
			Win32::DWORD openMode,
			Win32::DWORD pipeMode
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
			m_openMode |= Win32::FileFlagOverlapped; // Ensure we're overlapped
			InternalCreatePipe();
		}

		void operator=(const OverlappedNamedPipeServer& other)
		{
			Copy(other);
		}

		void operator=(OverlappedNamedPipeServer&& other) noexcept
		{
			Move(other);
		}

		void Connect(Async::OverlappedOp& oio)
		{
			if (not m_pipe)
				throw Error::Boring32Error("No valid pipe handle to connect");
			oio = Async::OverlappedOp();
			bool succeeded = Win32::ConnectNamedPipe(m_pipe.GetHandle(), oio.GetOverlapped());
			oio.LastError(GetLastError());
			if (not succeeded and oio.LastError() != Win32::ErrorCodes::IoPending)
				throw Error::Win32Error("ConnectNamedPipe() failed", oio.LastError());

			/*
			HANDLE out = nullptr;
			RegisterWaitForSingleObject(
				&out,
				oio.GetWaitableHandle(),
				Poop,//[](void* data, BYTE timedOut)->void {},
				this,
				INFINITE,
				WT_EXECUTEDEFAULT
			);
			*/
		}

		bool Connect(Async::OverlappedOp& op, std::nothrow_t) noexcept 
		try	
		{
			Connect(op);
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}

		void Write(const std::wstring& msg, Async::OverlappedIo& oio)
		{
			InternalWrite(msg, oio);
		}

		bool Write(const std::wstring& msg, Async::OverlappedIo& op, std::nothrow_t) noexcept 
		try
		{
			InternalWrite(msg, op);
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}

		void Read(const DWORD noOfCharacters, Async::OverlappedIo& oio)
		{
			InternalRead(noOfCharacters, oio);
		}

		bool Read(Win32::DWORD noOfCharacters, Async::OverlappedIo& oio, std::nothrow_t) noexcept 
		try
		{
			InternalRead(noOfCharacters, oio);
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}
		
		void InternalWrite(const std::wstring& msg, Async::OverlappedIo& oio)
		{
			if (not m_pipe)
				throw Error::Boring32Error("No pipe to write to");

			oio = Async::OverlappedIo();
			bool succeeded = Win32::WriteFile(
				m_pipe.GetHandle(),     // handle to pipe 
				&msg[0],                // buffer to write from 
				static_cast<Win32::DWORD>(msg.size() * sizeof(wchar_t)), // number of bytes to write 
				nullptr,          // number of bytes written 
				oio.GetOverlapped()       // overlapped I/O
			);
			oio.LastError(Win32::GetLastError());
			if (not succeeded and oio.LastError() != Win32::ErrorCodes::IoPending)
				throw Error::Win32Error("WriteFile() failed", oio.LastError());
		}

		void InternalRead(const Win32::DWORD noOfCharacters, Async::OverlappedIo& oio)
		{
			if (not m_pipe)
				throw Error::Boring32Error("No pipe to read from");

			oio = Async::OverlappedIo();
			oio.IoBuffer.resize(noOfCharacters);

			bool succeeded = Win32::ReadFile(
				m_pipe.GetHandle(),                             // pipe handle 
				&oio.IoBuffer[0],                               // buffer to receive reply 
				static_cast<Win32::DWORD>(oio.IoBuffer.size() * sizeof(wchar_t)),   // size of buffer, in bytes
				nullptr,                                        // number of bytes read 
				oio.GetOverlapped());                           // overlapped
			if (succeeded)
				return;

			oio.LastError(Win32::GetLastError());
			if (oio.LastError() != Win32::ErrorCodes::IoPending and oio.LastError() != Win32::ErrorCodes::MoreData)
				throw Error::Win32Error("ReadFile() failed", oio.LastError());
		}
	};
}
