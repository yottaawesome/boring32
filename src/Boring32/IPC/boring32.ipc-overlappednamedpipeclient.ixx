module;

#include <string>
#include <Windows.h>

export module boring32.ipc:overlappednamedpipeclient;
import :namedpipeclientbase;
import boring32.async;

export namespace Boring32::IPC
{
	class OverlappedNamedPipeClient : public NamedPipeClientBase
	{
		public:
			virtual ~OverlappedNamedPipeClient();
			OverlappedNamedPipeClient();
			OverlappedNamedPipeClient(const OverlappedNamedPipeClient& other);
			OverlappedNamedPipeClient(OverlappedNamedPipeClient&& other) noexcept;
			OverlappedNamedPipeClient(const std::wstring& name);

		public:
			virtual void operator=(const OverlappedNamedPipeClient& other);
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