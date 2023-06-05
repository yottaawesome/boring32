module;

#include <source_location>;

export module boring32.ipc:overlappednamedpipeserver;
import <stdexcept>;
import <iostream>;
import <string>;
import <win32.hpp>;
import boring32.raii;
import boring32.async;
import boring32.error;
import :namedpipeserverbase;

export namespace Boring32::IPC
{
	class OverlappedNamedPipeServer : public NamedPipeServerBase
	{
		public:
			virtual ~OverlappedNamedPipeServer()
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
				const DWORD size,
				const DWORD maxInstances, // PIPE_UNLIMITED_INSTANCES
				const std::wstring& sid,
				const bool isInheritable,
				const bool isLocalPipe
			) : NamedPipeServerBase(
				pipeName,
				size,
				maxInstances,
				sid,
				isInheritable,
				PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
				PIPE_TYPE_MESSAGE           // message type pipe 
				| PIPE_READMODE_MESSAGE     // message-read mode
				| PIPE_WAIT
				| (isLocalPipe
					? PIPE_REJECT_REMOTE_CLIENTS
					: PIPE_ACCEPT_REMOTE_CLIENTS)
			)
			{
				InternalCreatePipe();
			}

			OverlappedNamedPipeServer(
				const std::wstring& pipeName,
				const DWORD size,
				const DWORD maxInstances, // PIPE_UNLIMITED_INSTANCES
				const std::wstring& sid,
				const bool isInheritable,
				const DWORD openMode,
				const DWORD pipeMode
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
				m_openMode |= FILE_FLAG_OVERLAPPED; // Ensure we're overlapped
				InternalCreatePipe();
			}

		public:
			virtual void operator=(const OverlappedNamedPipeServer& other)
			{
				Copy(other);
			}

			virtual void operator=(OverlappedNamedPipeServer&& other) noexcept
			{
				Move(other);
			}

		public:
			virtual void Connect(Async::OverlappedOp& oio)
			{
				if (!m_pipe)
					throw Error::Boring32Error("No valid pipe handle to connect");
				oio = Async::OverlappedOp();
				bool succeeded = ConnectNamedPipe(m_pipe.GetHandle(), oio.GetOverlapped());
				oio.LastError(GetLastError());
				if (!succeeded && oio.LastError() != ERROR_IO_PENDING)
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

			virtual bool Connect(
				Async::OverlappedOp& op, 
				const std::nothrow_t&
			) noexcept try	
			{
				Connect(op);
				return true;
			}
			catch (const std::exception& ex)
			{
				std::wcerr << L" TEXT(__FUNSIG__) " << ex.what();
				return false;
			}

			virtual void Write(const std::wstring& msg, Async::OverlappedIo& oio)
			{
				InternalWrite(msg, oio);
			}

			virtual bool Write(
				const std::wstring& msg, 
				Async::OverlappedIo& op, const std::nothrow_t&
			) noexcept try
			{
				InternalWrite(msg, op);
				return true;
			}
			catch (const std::exception& ex)
			{
				std::wcerr
					<< L"OverlappedNamedPipeServer::Write(): "
					<< ex.what()
					<< std::endl;
				return false;
			}

			virtual void Read(const DWORD noOfCharacters, Async::OverlappedIo& oio)
			{
				InternalRead(noOfCharacters, oio);
			}

			virtual bool Read(
				const DWORD noOfCharacters, 
				Async::OverlappedIo& oio, 
				const std::nothrow_t&
			) noexcept try
			{
				InternalRead(noOfCharacters, oio);
				return true;
			}
			catch (const std::exception& ex)
			{
				std::wcerr
					<< L"OverlappedNamedPipeServer::Read(): "
					<< ex.what()
					<< std::endl;
				return false;
			}

		protected:
			virtual void InternalWrite(const std::wstring& msg, Async::OverlappedIo& oio)
			{
				if (!m_pipe)
					throw Error::Boring32Error("No pipe to write to");

				oio = Async::OverlappedIo();
				const bool succeeded = WriteFile(
					m_pipe.GetHandle(),     // handle to pipe 
					&msg[0],                // buffer to write from 
					static_cast<DWORD>(msg.size() * sizeof(wchar_t)), // number of bytes to write 
					nullptr,          // number of bytes written 
					oio.GetOverlapped()       // overlapped I/O
				);
				oio.LastError(GetLastError());
				if (!succeeded && oio.LastError() != ERROR_IO_PENDING)
					throw Error::Win32Error("WriteFile() failed", oio.LastError());
			}

			virtual void InternalRead(const DWORD noOfCharacters, Async::OverlappedIo& oio)
			{
				if (!m_pipe)
					throw Error::Boring32Error("No pipe to read from");

				oio = Async::OverlappedIo();
				oio.IoBuffer.resize(noOfCharacters);

				const bool succeeded = ReadFile(
					m_pipe.GetHandle(),                             // pipe handle 
					&oio.IoBuffer[0],                               // buffer to receive reply 
					static_cast<DWORD>(oio.IoBuffer.size() * sizeof(wchar_t)),   // size of buffer, in bytes
					nullptr,                                        // number of bytes read 
					oio.GetOverlapped());                           // overlapped
				oio.LastError(GetLastError());
				if (
					!succeeded
					&& oio.LastError() != ERROR_IO_PENDING
					&& oio.LastError() != ERROR_MORE_DATA
				)
				{
					throw Error::Win32Error("ReadFile() failed", oio.LastError());
				}
			}
	};
}
