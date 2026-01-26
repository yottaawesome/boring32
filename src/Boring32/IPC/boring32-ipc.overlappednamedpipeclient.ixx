export module boring32:ipc.overlappednamedpipeclient;
import std;
import :win32;
import :error;
import :async;
import :ipc.namedpipeclientbase;

export namespace Boring32::IPC
{
	struct OverlappedNamedPipeClient final : NamedPipeClientBase
	{
		OverlappedNamedPipeClient() = default;
		OverlappedNamedPipeClient(const std::wstring& name)
			: NamedPipeClientBase(name, Win32::FileFlagOverlapped)
		{ }

		void Write(std::wstring_view msg, Async::OverlappedIo& oio)
		{
			InternalWrite(msg, oio);
		}

		bool Write(std::wstring_view msg, Async::OverlappedIo& op, std::nothrow_t) noexcept 
		try
		{
			InternalWrite(msg, op);
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}

		void Read(const Win32::DWORD noOfCharacters, Async::OverlappedIo& oio)
		{
			return InternalRead(noOfCharacters, oio);
		}

		bool Read(const Win32::DWORD noOfCharacters, Async::OverlappedIo& op, std::nothrow_t) noexcept 
		try
		{
			InternalRead(noOfCharacters, op);
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}

		private :
		void InternalWrite(std::wstring_view msg, Async::OverlappedIo& oio)
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
				throw Error::Win32Error(oio.LastError(), "WriteFile() failed");
		}

		void InternalRead(const Win32::DWORD noOfCharacters, Async::OverlappedIo& oio)
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
					throw Error::Win32Error(oio.LastError(), "ReadFile() failed");
		}
	};
}