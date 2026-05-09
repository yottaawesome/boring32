export module boring32:ipc.overlappednamedpipeclient;
import std;
import :win32;
import :error;
import :async;
import :ipc.namedpipeclientbase;

export namespace Boring32::IPC
{
	class OverlappedNamedPipeClient final : public NamedPipeClientBase
	{
	public:
		OverlappedNamedPipeClient() = default;
		OverlappedNamedPipeClient(const std::wstring& name)
			: NamedPipeClientBase(name, Win32::FileFlagOverlapped)
		{ }

		void Write(std::wstring_view msg, Async::OverlappedIo& oio)
		{
			InternalWrite(msg, oio);
		}

		auto TryWrite(
			std::wstring_view msg, 
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

		void Read(const Win32::DWORD noOfCharacters, Async::OverlappedIo& oio)
		{
			return InternalRead(noOfCharacters, oio);
		}

		auto TryRead(
			Win32::DWORD noOfCharacters, 
			Async::OverlappedIo& op
		) noexcept -> bool
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
			auto succeeded = 
				Win32::WriteFile(
					m_handle.GetHandle(),
					&msg[0],
					static_cast<Win32::DWORD>(msg.size() * sizeof(wchar_t)),
					nullptr,
					oio.GetOverlapped()
				);
			oio.LastError(Win32::GetLastError());
			if (not succeeded and oio.LastError() != Win32::ErrorCodes::IoPending)
				throw Error::Win32Error{oio.LastError(), "WriteFile() failed"};
		}

		void InternalRead(Win32::DWORD noOfCharacters, Async::OverlappedIo& oio)
		{
			if (not m_handle)
				throw Error::Boring32Error("No pipe to read from");

			oio = Async::OverlappedIo();
			oio.IoBuffer.resize(noOfCharacters);
			// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-readfile
			auto succeeded = 
				Win32::ReadFile(
					m_handle.GetHandle(),
					&oio.IoBuffer[0],
					static_cast<Win32::DWORD>(oio.IoBuffer.size() * sizeof(wchar_t)),
					nullptr,
					oio.GetOverlapped()
				);
			oio.LastError(Win32::GetLastError());

			if (not succeeded)
				if (oio.LastError() != Win32::ErrorCodes::IoPending and oio.LastError() != Win32::ErrorCodes::MoreData)
					throw Error::Win32Error{oio.LastError(), "ReadFile() failed"};
		}
	};
}