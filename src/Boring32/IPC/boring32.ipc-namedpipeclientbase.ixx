export module boring32.ipc:namedpipeclientbase;
import std;
import <win32.hpp>;

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
				const DWORD fileAttributes
			) : m_pipeName(name),
				m_fileAttributes(fileAttributes)
			{ }

		public:
			virtual void SetMode(const DWORD pipeMode)
			{
				// https://docs.microsoft.com/en-us/windows/win32/api/namedpipeapi/nf-namedpipeapi-setnamedpipehandlestate?redirectedfrom=MSDN
				//PIPE_READMODE_MESSAGE or PIPE_READMODE_BYTE
				DWORD passedPipeMode = pipeMode;
				const bool succeeded = SetNamedPipeHandleState(
					m_handle.GetHandle(),    // pipe handle 
					&passedPipeMode,  // new pipe mode 
					nullptr,     // don't set maximum bytes 
					nullptr);    // don't set maximum time 
				if (!succeeded)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("SetNamedPipeHandleState() failed", lastError);
				}
			}

			virtual void Connect(const DWORD timeout)
			{
				static std::wstring pipePrefix = LR"(\\.\pipe\)";
				if (m_pipeName.starts_with(pipePrefix) == false)
					m_pipeName = pipePrefix + m_pipeName;

				// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew
				m_handle = CreateFileW(
					m_pipeName.c_str(), // pipe name 
					GENERIC_READ | GENERIC_WRITE,// read and write access 
					0,					// no sharing 
					nullptr,			// default security attributes
					OPEN_EXISTING,		// opens existing pipe 
					m_fileAttributes,	// attributes 
					nullptr				// no template file
				);
				if (m_handle == INVALID_HANDLE_VALUE)
				{
					if (GetLastError() != ERROR_PIPE_BUSY || timeout == 0)
					{
						const auto lastError = GetLastError();
						throw Error::Win32Error("Failed to connect client pipe", lastError);
					}
					if (WaitNamedPipeW(m_pipeName.c_str(), timeout) == false)
					{
						const auto lastError = GetLastError();
						throw Error::Win32Error("Timed out trying to connect client pipe", lastError);
					}
				}
			}

			virtual bool Connect(
				const DWORD timeout, 
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

			virtual DWORD UnreadCharactersRemaining() const
			{
				if (!m_handle)
					throw Error::Boring32Error("No pipe to read from");
				DWORD bytesLeft = 0;
				const bool succeeded = PeekNamedPipe(
					m_handle.GetHandle(),
					nullptr,
					0,
					nullptr,
					nullptr,
					&bytesLeft
				);
				if (!succeeded)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("PeekNamedPipe() failed", lastError);
				}

				return bytesLeft / sizeof(wchar_t);
			}

			virtual void Flush()
			{
				if (!m_handle)
					throw Error::Boring32Error("No pipe to flush");
				if (FlushFileBuffers(m_handle.GetHandle()) == false)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("FlushFileBuffers() failed", lastError);
				}
			}

			virtual void CancelCurrentThreadIo()
			{
				if (!m_handle)
					throw Error::Boring32Error("Pipe is nullptr");
				if (!CancelIo(m_handle.GetHandle()))
				{
					const auto lastError = GetLastError();
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

			virtual void CancelCurrentProcessIo(OVERLAPPED* overlapped)
			{
				if (!m_handle)
					throw Error::Boring32Error("pipe is nullptr");
				if (!CancelIoEx(m_handle.GetHandle(), overlapped))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("CancelIo() failed", lastError);

				}
			}

			virtual bool CancelCurrentProcessIo(
				OVERLAPPED* overlapped, 
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
			DWORD m_fileAttributes = 0;
	};
}