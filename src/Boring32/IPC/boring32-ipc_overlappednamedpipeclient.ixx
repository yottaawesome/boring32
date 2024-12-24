export module boring32:ipc_overlappednamedpipeclient;
import boring32.shared;
import :error;
import :async;
import :ipc_namedpipeclientbase;

export namespace Boring32::IPC
{
	struct OverlappedNamedPipeClient : NamedPipeClientBase
	{
		virtual ~OverlappedNamedPipeClient() = default;
		OverlappedNamedPipeClient() = default;
		OverlappedNamedPipeClient(const OverlappedNamedPipeClient& other) = default;
		OverlappedNamedPipeClient(OverlappedNamedPipeClient&& other) noexcept = default;
		OverlappedNamedPipeClient(const std::wstring& name)
			: NamedPipeClientBase(name, Win32::FileFlagOverlapped)
		{ }

		virtual OverlappedNamedPipeClient& operator=(const OverlappedNamedPipeClient& other) = default;
		virtual OverlappedNamedPipeClient& operator=(OverlappedNamedPipeClient&& other) noexcept = default;

		virtual void Write(std::wstring_view msg, Async::OverlappedIo& oio)
		{
			InternalWrite(msg, oio);
		}

		virtual bool Write(std::wstring_view msg, Async::OverlappedIo& op, std::nothrow_t) noexcept 
		try
		{
			InternalWrite(msg, op);
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}

		virtual void Read(const Win32::DWORD noOfCharacters, Async::OverlappedIo& oio)
		{
			return InternalRead(noOfCharacters, oio);
		}

		virtual bool Read(const Win32::DWORD noOfCharacters, Async::OverlappedIo& op, std::nothrow_t) noexcept 
		try
		{
			InternalRead(noOfCharacters, op);
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}

		protected:
		virtual void InternalWrite(std::wstring_view msg, Async::OverlappedIo& oio)
		{
			if (not m_handle)
				throw Error::Boring32Error("No pipe to write to");

			oio = Async::OverlappedIo();
			// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
			const bool succeeded = Win32::WriteFile(
				m_handle.GetHandle(),							// pipe handle 
				&msg[0],										// message 
				static_cast<Win32::DWORD>(msg.size() * sizeof(wchar_t)),	// message length, in bytes
				nullptr,										// out bytes written 
				oio.GetOverlapped());							// overlapped 
			oio.LastError(Win32::GetLastError());
			if (not succeeded and oio.LastError() != Win32::ErrorCodes::IoPending)
				throw Error::Win32Error("WriteFile() failed", oio.LastError());
		}

		virtual void InternalRead(const Win32::DWORD noOfCharacters, Async::OverlappedIo& oio)
		{
			if (not m_handle)
				throw Error::Boring32Error("No pipe to read from");

			oio = Async::OverlappedIo();
			oio.IoBuffer.resize(noOfCharacters);
			// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-readfile
			const bool succeeded = Win32::ReadFile(
				m_handle.GetHandle(),							// pipe handle 
				&oio.IoBuffer[0],								// buffer to receive reply 
				static_cast<Win32::DWORD>(oio.IoBuffer.size() * sizeof(wchar_t)),	// size of buffer, in bytes 
				nullptr,										// number of bytes read 
				oio.GetOverlapped());							// overlapped
			oio.LastError(Win32::GetLastError());

			if (not succeeded)
				if (oio.LastError() != Win32::ErrorCodes::IoPending and oio.LastError() != Win32::ErrorCodes::MoreData)
					throw Error::Win32Error("ReadFile() failed", oio.LastError());
		}
	};
}