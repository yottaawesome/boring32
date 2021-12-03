module;

#include <string>
#include <Windows.h>
#include "include/Async/OverlappedIo.hpp"
#include "include/Async/Pipes/NamedPipeClientBase.hpp"

export module boring32.ipc.overlappednamedpipeclient;

export namespace Boring32::IPC
{
	class OverlappedNamedPipeClient : public Async::NamedPipeClientBase
	{
		public:
			virtual ~OverlappedNamedPipeClient();
			OverlappedNamedPipeClient();
			OverlappedNamedPipeClient(const std::wstring& name);

			OverlappedNamedPipeClient(const OverlappedNamedPipeClient& other);
			virtual void operator=(const OverlappedNamedPipeClient& other);

			OverlappedNamedPipeClient(OverlappedNamedPipeClient&& other) noexcept;
			virtual void operator=(OverlappedNamedPipeClient&& other) noexcept;

		public:
			virtual void Write(std::wstring_view msg, Async::OverlappedIo& op);
			virtual bool Write(std::wstring_view msg, Async::OverlappedIo& op, const std::nothrow_t&) noexcept;
			virtual void Read(const DWORD noOfCharacters, Async::OverlappedIo& op);
			virtual bool Read(const DWORD noOfCharacters, Async::OverlappedIo& op, const std::nothrow_t&) noexcept;

		protected:
			virtual void InternalWrite(std::wstring_view msg, Async::OverlappedIo& op);
			virtual void InternalRead(const DWORD noOfCharacters, Async::OverlappedIo& op);
	};
}