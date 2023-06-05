module;

#include <source_location>;

export module boring32.ipc:namedpipeserverbase;
import <string>;
import <iostream>;
import <format>;
import <win32.hpp>;
import boring32.error;
import boring32.raii;

export namespace Boring32::IPC
{
	class NamedPipeServerBase
	{
		// example of DACL/SID strings: "D:(A;;GA;;;BA)(A;;GR;;;BU)" and "D:(A;;GRGW;;;BU)"
		public:
			virtual ~NamedPipeServerBase()
			{
				Close();
			}

			NamedPipeServerBase() = default;
			

			NamedPipeServerBase(const NamedPipeServerBase& other)
				: NamedPipeServerBase()
			{
				Copy(other);
			}

			virtual void operator=(const NamedPipeServerBase& other)
			{
				Copy(other);
			}

			NamedPipeServerBase(NamedPipeServerBase&& other) noexcept
				: NamedPipeServerBase()
			{
				Move(other);
			}

			virtual void operator=(NamedPipeServerBase&& other) noexcept
			{
				Move(other);
			}

		public:
			NamedPipeServerBase(
				const std::wstring& pipeName, 
				const DWORD size,
				const DWORD maxInstances, // PIPE_UNLIMITED_INSTANCES
				const std::wstring& sid,
				const bool isInheritable,
				const bool isLocalPipe
			) : m_pipeName(pipeName),
				m_size(size),
				m_maxInstances(maxInstances),
				m_sid(sid),
				m_isInheritable(isInheritable),
				m_openMode(
					PIPE_ACCESS_DUPLEX          // read/write access
				),
				m_pipeMode(
					PIPE_TYPE_MESSAGE           // message type pipe 
					| PIPE_READMODE_MESSAGE     // message-read mode
					| PIPE_WAIT
				)
			{
				if (isLocalPipe)
					m_pipeMode |= PIPE_ACCEPT_REMOTE_CLIENTS;
				else
					m_pipeMode |= PIPE_REJECT_REMOTE_CLIENTS;
			}

			NamedPipeServerBase(
				const std::wstring& pipeName,
				const DWORD size,
				const DWORD maxInstances, // PIPE_UNLIMITED_INSTANCES
				const std::wstring& sid,
				const bool isInheritable,
				const DWORD openMode,
				const DWORD pipeMode
			) : m_pipeName(pipeName),
				m_size(size),
				m_maxInstances(maxInstances),
				m_sid(sid),
				m_isInheritable(isInheritable),
				m_openMode(openMode),
				m_pipeMode(pipeMode)
			{ }

		public:
			virtual void Close()
			{
				Disconnect();
				m_pipe.Close();
			}

			virtual void Disconnect()
			{
				if (m_pipe != nullptr)
					DisconnectNamedPipe(m_pipe.GetHandle());
			}

			virtual void Flush()
			{
				if (!m_pipe)
					throw Error::Boring32Error("No pipe to flush");
				if (!FlushFileBuffers(m_pipe.GetHandle()))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("Flush() failed", lastError);
				}
			}

			virtual RAII::Win32Handle& GetInternalHandle()
			{
				return m_pipe;
			}

			virtual std::wstring GetName() const
			{
				return m_pipeName;
			}

			virtual DWORD GetSize() const
			{
				return m_size;
			}

			virtual DWORD GetMaxInstances() const
			{
				return m_maxInstances;
			}

			virtual DWORD GetPipeMode() const
			{
				return m_pipeMode;
			}

			virtual DWORD GetOpenMode() const
			{
				return m_openMode;
			}

			virtual DWORD UnreadCharactersRemaining() const
			{
				DWORD charactersRemaining = 0;
				InternalUnreadCharactersRemaining(charactersRemaining, true);
				return charactersRemaining;
			}

			virtual bool UnreadCharactersRemaining(
				DWORD& charactersRemaining, 
				std::nothrow_t
			) const noexcept 
			{
				return InternalUnreadCharactersRemaining(charactersRemaining, false);
			}

			virtual void CancelCurrentThreadIo()
			{
				if (!m_pipe)
					throw Error::Boring32Error("pipe is nullptr");
				if (!CancelIo(m_pipe.GetHandle()))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("CancelIo() failed", lastError);
				}
			}

			virtual bool CancelCurrentThreadIo(
				const std::nothrow_t&
			) noexcept try
			{
				CancelCurrentThreadIo();
				return true;
			}
			catch (const std::exception& ex)
			{
				std::wcerr << std::format(
					"CancelCurrentThreadIo failed: {}\n", ex.what()
				).c_str();
				return false;
			}

			virtual void CancelCurrentProcessIo(OVERLAPPED* overlapped)
			{
				if (!m_pipe)
					throw Error::Boring32Error("pipe is nullptr");
				if (!CancelIoEx(m_pipe.GetHandle(), overlapped))
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
			catch (const std::exception& ex)
			{
				std::wcerr << std::format(
					"CancelCurrentProcessIo() failed: {}\n", 
					ex.what()
				).c_str();
				return false;
			}

		protected:
			virtual void InternalCreatePipe()
			{
				if (!m_pipeName.starts_with(L"\\\\.\\pipe\\"))
					m_pipeName = L"\\\\.\\pipe\\" + m_pipeName;

				SECURITY_ATTRIBUTES sa{
					.nLength = sizeof(sa),
					.bInheritHandle = m_isInheritable
				};
				if (!m_sid.empty())
				{
					const bool converted = ConvertStringSecurityDescriptorToSecurityDescriptorW(
						m_sid.c_str(),
						SDDL_REVISION_1,
						&sa.lpSecurityDescriptor,
						nullptr
					);
					if (!converted)
					{
						const auto lastError = GetLastError();
						throw Error::Win32Error("Failed to convert security descriptor", lastError);
					}
				}

				// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-createnamedpipea
				m_pipe = CreateNamedPipeW(
					m_pipeName.c_str(),             // pipe name
					m_openMode,
					m_pipeMode,
					m_maxInstances,                 // max. instances  
					m_size,                         // output buffer size 
					m_size,                         // input buffer size 
					0,                              // client time-out 
					!m_sid.empty() ? &sa : nullptr
				);
				if (!m_sid.empty())
					LocalFree(sa.lpSecurityDescriptor);
				if (!m_pipe)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("Failed to create named pipe", lastError);
				}
			}

			virtual void Copy(const NamedPipeServerBase& other)
			{
				Close();
				m_pipe = other.m_pipe;
				m_pipeName = other.m_pipeName;
				m_size = other.m_size;
				m_maxInstances = other.m_maxInstances;
				m_sid = other.m_sid;
				m_openMode = other.m_openMode;
				m_pipeMode = other.m_pipeMode;
			}

			virtual void Move(NamedPipeServerBase& other) noexcept
			{
				Close();
				m_pipeName = std::move(other.m_pipeName);
				m_size = other.m_size;
				m_maxInstances = other.m_maxInstances;
				m_sid = std::move(other.m_sid);
				m_openMode = other.m_openMode;
				m_pipeMode = other.m_pipeMode;
				if (other.m_pipe != nullptr)
					m_pipe = std::move(other.m_pipe);
			}

			virtual bool InternalUnreadCharactersRemaining(
				DWORD& charactersRemaining, 
				const bool throwOnError
			) const
			{
				if (!m_pipe)
					return false;
				charactersRemaining = 0;
				bool succeeded = PeekNamedPipe(
					m_pipe.GetHandle(),
					nullptr,
					0,
					nullptr,
					nullptr,
					&charactersRemaining
				);
				if (!succeeded)
				{
					if (throwOnError)
					{
						const auto lastError = GetLastError();
						throw Error::Win32Error("PeekNamedPipe() failed", lastError);
					}
					return false;
				}

				charactersRemaining /= sizeof(wchar_t);
				return true;
			}

		protected:
			RAII::Win32Handle m_pipe;
			std::wstring m_pipeName;
			std::wstring m_sid;
			DWORD m_size = 0;
			DWORD m_maxInstances = 0;
			bool m_isInheritable = false;
			DWORD m_pipeMode = 0;
			DWORD m_openMode = 0;
	};
}
