module;

#include <string>
#include <Windows.h>
#include "include/Async/OverlappedOp.hpp"
#include "include/Async/OverlappedIo.hpp"

export module boring32.ipc.overlappednamedpipeserver;
import boring32.ipc.namedpipeserverbase;
import boring32.raii.win32handle;

export namespace Boring32::IPC
{
	class OverlappedNamedPipeServer : public Async::NamedPipeServerBase
	{
		public:
			virtual ~OverlappedNamedPipeServer();
			OverlappedNamedPipeServer();
			OverlappedNamedPipeServer(
				const std::wstring& pipeName, 
				const DWORD size,
				const DWORD maxInstances, // PIPE_UNLIMITED_INSTANCES
				const std::wstring& sid,
				const bool isInheritable,
				const bool isLocalPipe
			);
			OverlappedNamedPipeServer(
				const std::wstring& pipeName,
				const DWORD size,
				const DWORD maxInstances, // PIPE_UNLIMITED_INSTANCES
				const std::wstring& sid,
				const bool isInheritable,
				const DWORD openMode,
				const DWORD pipeMode
			);

			OverlappedNamedPipeServer(const OverlappedNamedPipeServer& other);
			virtual void operator=(const OverlappedNamedPipeServer& other);

			OverlappedNamedPipeServer(OverlappedNamedPipeServer&& other) noexcept;
			virtual void operator=(OverlappedNamedPipeServer&& other) noexcept;

		public:
			virtual void Connect(Async::OverlappedOp& op);
			virtual bool Connect(Async::OverlappedOp& op, std::nothrow_t) noexcept;
			virtual void Write(const std::wstring& msg, Async::OverlappedIo& oio);
			virtual bool Write(const std::wstring& msg, Async::OverlappedIo& op, std::nothrow_t) noexcept;
			virtual void Read(const DWORD noOfCharacters, Async::OverlappedIo& oio);
			virtual bool Read(const DWORD noOfCharacters, Async::OverlappedIo& oio, std::nothrow_t) noexcept;

		protected:
			virtual void InternalWrite(const std::wstring& msg, Async::OverlappedIo& oio);
			virtual void InternalRead(const DWORD noOfCharacters, Async::OverlappedIo& oio);
	};
}
