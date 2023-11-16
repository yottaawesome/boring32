export module boring32.ipc:namedpipeclientbase;
import <string>;
import <iostream>;
import boring32.win32;
import boring32.raii;
import boring32.error;

export namespace Boring32::IPC
{
	class NamedPipeClientBase
	{
		// The six
		public:
			virtual ~NamedPipeClientBase() = default;
			NamedPipeClientBase() = default;
			NamedPipeClientBase(const NamedPipeClientBase& other) = default;
			NamedPipeClientBase(NamedPipeClientBase&& other) noexcept = default;
			virtual NamedPipeClientBase& operator=(const NamedPipeClientBase& other) = default;
			virtual NamedPipeClientBase& operator=(NamedPipeClientBase&& other) noexcept = default;

		public:
			NamedPipeClientBase(
				const std::wstring& name,
				const Win32::DWORD fileAttributes
			) : m_pipeName(name),
				m_fileAttributes(fileAttributes)
			{ }

		public:
			virtual void SetMode(const Win32::DWORD pipeMode)
			{
				// https://docs.microsoft.com/en-us/windows/win32/api/namedpipeapi/nf-namedpipeapi-setnamedpipehandlestate?redirectedfrom=MSDN
				//PIPE_READMODE_MESSAGE or PIPE_READMODE_BYTE
				Win32::DWORD passedPipeMode = pipeMode;
				const bool succeeded = Win32::SetNamedPipeHandleState(
					m_handle.GetHandle(),    // pipe handle 
					&passedPipeMode,  // new pipe mode 
					nullptr,     // don't set maximum bytes 
					nullptr);    // don't set maximum time 
				if (!succeeded)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("SetNamedPipeHandleState() failed", lastError);
				}
			}

			virtual void Connect(const Win32::DWORD timeout)
			{
				static std::wstring pipePrefix = LR"(\\.\pipe\)";
				if (!m_pipeName.starts_with(pipePrefix))
					m_pipeName = pipePrefix + m_pipeName;

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
					if (Win32::GetLastError() != Win32::ErrorCodes::PipeBusy || timeout == 0)
					{
						const auto lastError = Win32::GetLastError();
						throw Error::Win32Error("Failed to connect client pipe", lastError);
					}
					if (Win32::WaitNamedPipeW(m_pipeName.c_str(), timeout) == false)
					{
						const auto lastError = Win32::GetLastError();
						throw Error::Win32Error("Timed out trying to connect client pipe", lastError);
					}
				}
			}

			virtual bool Connect(
				const Win32::DWORD timeout,
				const std::nothrow_t&
			) noexcept try
			{
				Connect(timeout);
				return true;
			}
			catch (const std::exception&)
			{
				// ICE -- seems to only happen in a catch
				/*std::wcerr
					<< __FUNCSIG__ << L" failed: "
					<< ex.what()
					<< std::endl;*/
				return false;
			}

			virtual void Close()
			{
				m_handle.Close();
			}

			virtual Win32::DWORD UnreadCharactersRemaining() const
			{
				if (!m_handle)
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
				if (!succeeded)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("PeekNamedPipe() failed", lastError);
				}

				return bytesLeft / sizeof(wchar_t);
			}

			virtual void Flush()
			{
				if (!m_handle)
					throw Error::Boring32Error("No pipe to flush");
				if (!Win32::FlushFileBuffers(m_handle.GetHandle()))
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("FlushFileBuffers() failed", lastError);
				}
			}

			virtual void CancelCurrentThreadIo()
			{
				if (!m_handle)
					throw Error::Boring32Error("Pipe is nullptr");
				if (!Win32::CancelIo(m_handle.GetHandle()))
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("CancelIo failed", lastError);
				}
			}

			virtual bool CancelCurrentThreadIo(const std::nothrow_t&) noexcept try
			{
				CancelCurrentThreadIo();
				return true;
			}
			catch (const std::exception&)
			{
				// ICE
				//std::wcerr << __FUNCSIG__ << L" failed: " << ex.what() << std::endl;
				return false;
			}

			virtual void CancelCurrentProcessIo(Win32::OVERLAPPED* overlapped)
			{
				if (!m_handle)
					throw Error::Boring32Error("pipe is nullptr");
				if (!Win32::CancelIoEx(m_handle.GetHandle(), overlapped))
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("CancelIo() failed", lastError);

				}
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