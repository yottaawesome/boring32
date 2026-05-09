export module boring32:ipc.overlappednamedpipeserver;
import std;
import :win32;
import :raii;
import :error;
import :async;
import :ipc.namedpipeserverbase;

export namespace Boring32::IPC
{
	class OverlappedNamedPipeServer final : public NamedPipeServerBase
	{
	public:
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
		auto operator=(const OverlappedNamedPipeServer& other) -> OverlappedNamedPipeServer&
		{
			Copy(other);
			return *this;
		}

		OverlappedNamedPipeServer(OverlappedNamedPipeServer&& other) noexcept
			: NamedPipeServerBase(std::move(other))
		{
			InternalCreatePipe();
		}
		auto operator=(OverlappedNamedPipeServer&& other) noexcept -> OverlappedNamedPipeServer&
		{
			Move(other);
			return *this;
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

		void Connect(Async::OverlappedOp& oio)
		{
			if (not m_pipe)
				throw Error::Boring32Error("No valid pipe handle to connect");
			oio = Async::OverlappedOp();
			bool succeeded = Win32::ConnectNamedPipe(m_pipe.GetHandle(), oio.GetOverlapped());
			oio.LastError(Win32::GetLastError());
			if (not succeeded and oio.LastError() != Win32::ErrorCodes::IoPending)
				throw Error::Win32Error{oio.LastError(), "ConnectNamedPipe() failed"};

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

		auto Connect(Async::OverlappedOp& op, std::nothrow_t) noexcept -> bool
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

		auto TryWrite(
			const std::wstring& msg, 
			Async::OverlappedIo& op
		) noexcept -> bool
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

		auto TryRead(
			Win32::DWORD noOfCharacters, 
			Async::OverlappedIo& oio
		) noexcept -> bool
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
			auto succeeded = 
				Win32::WriteFile(
					m_pipe.GetHandle(),
					&msg[0],
					static_cast<Win32::DWORD>(msg.size() * sizeof(wchar_t)),
					nullptr,
					oio.GetOverlapped()
				);
			oio.LastError(Win32::GetLastError());
			if (not succeeded and oio.LastError() != Win32::ErrorCodes::IoPending)
				throw Error::Win32Error{oio.LastError(), "WriteFile() failed"};
		}

		void InternalRead(const Win32::DWORD noOfCharacters, Async::OverlappedIo& oio)
		{
			if (not m_pipe)
				throw Error::Boring32Error("No pipe to read from");

			oio = Async::OverlappedIo();
			oio.IoBuffer.resize(noOfCharacters);

			auto succeeded = 
				Win32::ReadFile(
					m_pipe.GetHandle(),
					&oio.IoBuffer[0],
					static_cast<Win32::DWORD>(oio.IoBuffer.size() * sizeof(wchar_t)),
					nullptr,
					oio.GetOverlapped()
				);
			if (succeeded)
				return;

			oio.LastError(Win32::GetLastError());
			if (oio.LastError() != Win32::ErrorCodes::IoPending and oio.LastError() != Win32::ErrorCodes::MoreData)
				throw Error::Win32Error{oio.LastError(), "ReadFile() failed"};
		}
	};
}
