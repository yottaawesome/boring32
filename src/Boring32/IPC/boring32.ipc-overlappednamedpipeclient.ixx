export module boring32.ipc:overlappednamedpipeclient;
import :namedpipeclientbase;
import boring32.async;
import <string>;
import <win32.hpp>;

export namespace Boring32::IPC
{
	class OverlappedNamedPipeClient : public NamedPipeClientBase
	{
		public:
			virtual ~OverlappedNamedPipeClient() = default;
			OverlappedNamedPipeClient() = default;
			OverlappedNamedPipeClient(const OverlappedNamedPipeClient& other) = default;
			OverlappedNamedPipeClient(OverlappedNamedPipeClient&& other) noexcept = default;
			OverlappedNamedPipeClient(const std::wstring& name);

		public:
			virtual OverlappedNamedPipeClient& operator=(const OverlappedNamedPipeClient& other) = default;
			virtual OverlappedNamedPipeClient& operator=(OverlappedNamedPipeClient&& other) noexcept = default;

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