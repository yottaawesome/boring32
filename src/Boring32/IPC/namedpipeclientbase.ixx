export module boring32:ipc.namedpipeclientbase;
import std;
import :win32;
import :raii;
import :error;

export namespace Boring32::IPC
{
	class NamedPipeClientBase
	{
	public:
		NamedPipeClientBase() = default;
		NamedPipeClientBase(const NamedPipeClientBase& other) = default;
		NamedPipeClientBase(NamedPipeClientBase&& other) noexcept = default;
		auto operator=(const NamedPipeClientBase& other) -> NamedPipeClientBase& = default;
		auto operator=(NamedPipeClientBase&& other) noexcept -> NamedPipeClientBase& = default;

		NamedPipeClientBase(
			const std::wstring& name,
			Win32::DWORD fileAttributes
		) : m_pipeName(name),
			m_fileAttributes(fileAttributes)
		{ }

		void SetMode(this auto&& self, Win32::DWORD pipeMode)
		{
			// https://docs.microsoft.com/en-us/windows/win32/api/namedpipeapi/nf-namedpipeapi-setnamedpipehandlestate?redirectedfrom=MSDN
			//PIPE_READMODE_MESSAGE or PIPE_READMODE_BYTE
			auto passedPipeMode = pipeMode;
			auto succeeded = Win32::SetNamedPipeHandleState(
				self.m_handle.GetHandle(),    // pipe handle 
				&passedPipeMode,  // new pipe mode 
				nullptr,     // don't set maximum bytes 
				nullptr);    // don't set maximum time 
			if (not succeeded)
				throw Error::Win32Error{Win32::GetLastError(), "SetNamedPipeHandleState() failed"};
		}

		void Connect(this auto&& self, Win32::DWORD timeout)
		{
			constexpr auto pipePrefix = std::wstring_view{LR"(\\.\pipe\)"};
			if (not self.m_pipeName.starts_with(pipePrefix))
				self.m_pipeName = std::format(L"{}{}", pipePrefix, self.m_pipeName);
			// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew
			self.m_handle = Win32::CreateFileW(
				self.m_pipeName.c_str(), // pipe name 
				Win32::GenericRead | Win32::GenericWrite,// read and write access 
				0,					// no sharing 
				nullptr,			// default security attributes
				Win32::_OPEN_EXISTING,		// opens existing pipe 
				self.m_fileAttributes,	// attributes 
				nullptr				// no template file
			);
			if (self.m_handle == Win32::InvalidHandleValue)
			{
				if (auto lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::PipeBusy or timeout == 0)
					throw Error::Win32Error{lastError, "Failed to connect client pipe"};
				if (not Win32::WaitNamedPipeW(self.m_pipeName.c_str(), timeout))
					throw Error::Win32Error{Win32::GetLastError(), "Timed out trying to connect client pipe"};
			}
		}

		auto TryConnect(this auto&& self, Win32::DWORD timeout) noexcept -> bool
		try
		{
			self.Connect(timeout);
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}

		void Close(this auto&& self)
		{
			self.m_handle.Close();
		}

		auto UnreadCharactersRemaining(this auto&& self) -> Win32::DWORD
		{
			if (not self.m_handle)
				throw Error::Boring32Error("No pipe to read from");
			auto bytesLeft = Win32::DWORD{};
			auto succeeded = Win32::PeekNamedPipe(
				self.m_handle.GetHandle(),
				nullptr,
				0,
				nullptr,
				nullptr,
				&bytesLeft
			);
			if (not succeeded)
				throw Error::Win32Error{Win32::GetLastError(), "PeekNamedPipe() failed"};

			return bytesLeft / sizeof(wchar_t);
		}

		void Flush(this auto&& self)
		{
			if (not self.m_handle)
				throw Error::Boring32Error("No pipe to flush");
			if (not Win32::FlushFileBuffers(self.m_handle.GetHandle()))
				throw Error::Win32Error{Win32::GetLastError(), "FlushFileBuffers() failed"};
		}

		void CancelCurrentThreadIo(this auto&& self)
		{
			if (not self.m_handle)
				throw Error::Boring32Error("Pipe is nullptr");
			if (not Win32::CancelIo(self.m_handle.GetHandle()))
				throw Error::Win32Error{Win32::GetLastError(), "CancelIo failed"};
		}

		auto CancelCurrentThreadIo(this auto&& self, const std::nothrow_t&) noexcept -> bool try
		{
			self.CancelCurrentThreadIo();
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}

		void CancelCurrentProcessIo(this auto&& self, Win32::OVERLAPPED* overlapped)
		{
			if (not self.m_handle)
				throw Error::Boring32Error("pipe is nullptr");
			if (not Win32::CancelIoEx(self.m_handle.GetHandle(), overlapped))
				throw Error::Win32Error{Win32::GetLastError(), "CancelIo() failed"};
		}

		auto TryCancelCurrentProcessIo(
			this auto&& self, 
			Win32::OVERLAPPED* overlapped 
		) noexcept -> bool 
		try
		{
			self.CancelCurrentProcessIo(overlapped);
			return true;
		}
		catch (const std::exception&)
		{
			// ICE -- seems to only happen in a catch
			//std::wcerr << __FUNCSIG__ << L" failed: " << ex.what() << std::endl;
			return false;
		}

	protected:
		RAII::SharedHandle m_handle;
		std::wstring m_pipeName;
		Win32::DWORD m_fileAttributes = 0;
	};
}