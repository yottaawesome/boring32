export module boring32:ipc.namedpipeclientbase;
import std;
import :win32;
import :raii;
import :error;

export namespace Boring32::IPC
{
	struct NamedPipeClientBase
	{
		virtual ~NamedPipeClientBase() = default;
		NamedPipeClientBase() = default;
		NamedPipeClientBase(const NamedPipeClientBase& other) = default;
		NamedPipeClientBase(NamedPipeClientBase&& other) noexcept = default;
		virtual NamedPipeClientBase& operator=(const NamedPipeClientBase& other) = default;
		virtual NamedPipeClientBase& operator=(NamedPipeClientBase&& other) noexcept = default;

		NamedPipeClientBase(
			const std::wstring& name,
			const Win32::DWORD fileAttributes
		) : m_pipeName(name),
			m_fileAttributes(fileAttributes)
		{ }

		virtual void SetMode(const Win32::DWORD pipeMode)
		{
			// https://docs.microsoft.com/en-us/windows/win32/api/namedpipeapi/nf-namedpipeapi-setnamedpipehandlestate?redirectedfrom=MSDN
			//PIPE_READMODE_MESSAGE or PIPE_READMODE_BYTE
			Win32::DWORD passedPipeMode = pipeMode;
			bool succeeded = Win32::SetNamedPipeHandleState(
				m_handle.GetHandle(),    // pipe handle 
				&passedPipeMode,  // new pipe mode 
				nullptr,     // don't set maximum bytes 
				nullptr);    // don't set maximum time 
			if (not succeeded)
				throw Error::Win32Error(Win32::GetLastError(), "SetNamedPipeHandleState() failed");
		}

		virtual void Connect(const Win32::DWORD timeout)
		{
			constexpr std::wstring_view pipePrefix = LR"(\\.\pipe\)";
			if (not m_pipeName.starts_with(pipePrefix))
				m_pipeName = std::format(L"{}{}", pipePrefix, m_pipeName);

			// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew
			m_handle = Win32::CreateFileW(
				m_pipeName.c_str(), // pipe name 
				Win32::GenericRead | Win32::GenericWrite,// read and write access 
				0,					// no sharing 
				nullptr,			// default security attributes
				Win32::_OPEN_EXISTING,		// opens existing pipe 
				m_fileAttributes,	// attributes 
				nullptr				// no template file
			);
			if (m_handle == Win32::InvalidHandleValue)
			{
				if (auto lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::PipeBusy || timeout == 0)
					throw Error::Win32Error(lastError, "Failed to connect client pipe");
				if (not Win32::WaitNamedPipeW(m_pipeName.c_str(), timeout))
					throw Error::Win32Error(Win32::GetLastError(), "Timed out trying to connect client pipe");
			}
		}

			virtual bool Connect(Win32::DWORD timeout, std::nothrow_t) noexcept 
			try
			{
				Connect(timeout);
				return true;
			}
			catch (const std::exception&)
			{
				return false;
			}

			virtual void Close()
			{
				m_handle.Close();
			}

			virtual Win32::DWORD UnreadCharactersRemaining() const
			{
				if (not m_handle)
					throw Error::Boring32Error("No pipe to read from");
				Win32::DWORD bytesLeft = 0;
				const bool succeeded = Win32::PeekNamedPipe(
					m_handle.GetHandle(),
					nullptr,
					0,
					nullptr,
					nullptr,
					&bytesLeft
				);
				if (not succeeded)
					throw Error::Win32Error(Win32::GetLastError(), "PeekNamedPipe() failed");

				return bytesLeft / sizeof(wchar_t);
			}

			virtual void Flush()
			{
				if (not m_handle)
					throw Error::Boring32Error("No pipe to flush");
				if (not Win32::FlushFileBuffers(m_handle.GetHandle()))
					throw Error::Win32Error(Win32::GetLastError(), "FlushFileBuffers() failed");
			}

			virtual void CancelCurrentThreadIo()
			{
				if (not m_handle)
					throw Error::Boring32Error("Pipe is nullptr");
				if (not Win32::CancelIo(m_handle.GetHandle()))
					throw Error::Win32Error(Win32::GetLastError(), "CancelIo failed");
			}

			virtual bool CancelCurrentThreadIo(const std::nothrow_t&) noexcept try
			{
				CancelCurrentThreadIo();
				return true;
			}
			catch (const std::exception&)
			{
				return false;
			}

			virtual void CancelCurrentProcessIo(Win32::OVERLAPPED* overlapped)
			{
				if (not m_handle)
					throw Error::Boring32Error("pipe is nullptr");
				if (not Win32::CancelIoEx(m_handle.GetHandle(), overlapped))
					throw Error::Win32Error(Win32::GetLastError(), "CancelIo() failed");
			}

			virtual bool CancelCurrentProcessIo(
				Win32::OVERLAPPED* overlapped,
				const std::nothrow_t&
			) noexcept try
			{
				CancelCurrentProcessIo(overlapped);
				return true;
			}
			catch (const std::exception&)
			{
				// ICE -- seems to only happen in a catch
				//std::wcerr << __FUNCSIG__ << L" failed: " << ex.what() << std::endl;
				return false;
			}

		protected:
			RAII::Win32Handle m_handle;
			std::wstring m_pipeName;
			Win32::DWORD m_fileAttributes = 0;
	};
}