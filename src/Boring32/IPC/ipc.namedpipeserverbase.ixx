export module boring32:ipc.namedpipeserverbase;
import std;
import :win32;
import :error;
import :raii;

export namespace Boring32::IPC
{
	struct NamedPipeServerBase
	{
		// example of DACL/SID strings: "D:(A;;GA;;;BA)(A;;GR;;;BU)" and "D:(A;;GRGW;;;BU)"
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

		NamedPipeServerBase(
			const std::wstring& pipeName, 
			const Win32::DWORD size,
			const Win32::DWORD maxInstances, // PIPE_UNLIMITED_INSTANCES
			const std::wstring& sid,
			const bool isInheritable,
			const bool isLocalPipe
		) : m_pipeName(pipeName),
			m_size(size),
			m_maxInstances(maxInstances),
			m_sid(sid),
			m_isInheritable(isInheritable),
			m_openMode(
				Win32::PipeAccessDuplex          // read/write access
			),
			m_pipeMode(
				Win32::PipeTypeMessage           // message type pipe 
				| Win32::PipeReadModeMessage     // message-read mode
				| Win32::PipeWait
			)
		{
			m_pipeMode |= (isLocalPipe ? Win32::PipeRejectRemoteClients : Win32::PipeAcceptRemoteClients);
		}

		NamedPipeServerBase(
			const std::wstring& pipeName,
			const Win32::DWORD size,
			const Win32::DWORD maxInstances, // PIPE_UNLIMITED_INSTANCES
			const std::wstring& sid,
			const bool isInheritable,
			const Win32::DWORD openMode,
			const Win32::DWORD pipeMode
		) : m_pipeName(pipeName),
			m_size(size),
			m_maxInstances(maxInstances),
			m_sid(sid),
			m_isInheritable(isInheritable),
			m_openMode(openMode),
			m_pipeMode(pipeMode)
		{ }

		virtual void Close()
		{
			Disconnect();
			m_pipe.Close();
		}

		virtual void Disconnect()
		{
			if (m_pipe)
				Win32::DisconnectNamedPipe(m_pipe.GetHandle());
		}

		virtual void Flush()
		{
			if (not m_pipe)
				throw Error::Boring32Error("No pipe to flush");
			if (not Win32::FlushFileBuffers(m_pipe.GetHandle()))
				throw Error::Win32Error(Win32::GetLastError(), "Flush() failed");
		}

		virtual RAII::Win32Handle& GetInternalHandle()
		{
			return m_pipe;
		}

		virtual std::wstring GetName() const
		{
			return m_pipeName;
		}

		virtual Win32::DWORD GetSize() const
		{
			return m_size;
		}

		virtual Win32::DWORD GetMaxInstances() const
		{
			return m_maxInstances;
		}

		virtual Win32::DWORD GetPipeMode() const
		{
			return m_pipeMode;
		}

		virtual Win32::DWORD GetOpenMode() const
		{
			return m_openMode;
		}

		virtual Win32::DWORD UnreadCharactersRemaining() const
		{
			Win32::DWORD charactersRemaining = 0;
			InternalUnreadCharactersRemaining(charactersRemaining, true);
			return charactersRemaining;
		}

		virtual bool UnreadCharactersRemaining(
			Win32::DWORD& charactersRemaining,
			std::nothrow_t
		) const noexcept 
		{
			return InternalUnreadCharactersRemaining(charactersRemaining, false);
		}

		virtual void CancelCurrentThreadIo()
		{
			if (not m_pipe)
				throw Error::Boring32Error("pipe is nullptr");
			if (not Win32::CancelIo(m_pipe.GetHandle()))
				throw Error::Win32Error(Win32::GetLastError(), "CancelIo() failed");
		}

		virtual bool CancelCurrentThreadIo(std::nothrow_t) noexcept 
		try
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
			if (not m_pipe)
				throw Error::Boring32Error("pipe is nullptr");
			if (not Win32::CancelIoEx(m_pipe.GetHandle(), overlapped))
				throw Error::Win32Error(Win32::GetLastError(), "CancelIo() failed");
		}

		virtual bool CancelCurrentProcessIo(Win32::OVERLAPPED* overlapped, std::nothrow_t) noexcept 
		try
		{
			CancelCurrentProcessIo(overlapped);
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}

		protected:
		virtual void InternalCreatePipe()
		{
			if (not m_pipeName.starts_with(L"\\\\.\\pipe\\"))
				m_pipeName = L"\\\\.\\pipe\\" + m_pipeName;

			Win32::SECURITY_ATTRIBUTES sa{
				.nLength = sizeof(sa),
				.bInheritHandle = m_isInheritable
			};
			if (not m_sid.empty())
			{
				bool converted = Win32::ConvertStringSecurityDescriptorToSecurityDescriptorW(
					m_sid.c_str(),
					Win32::SddlRevision1,
					&sa.lpSecurityDescriptor,
					nullptr
				);
				if (not converted)
					throw Error::Win32Error(Win32::GetLastError(), "Failed to convert security descriptor");
			}

			// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-createnamedpipea
			m_pipe = Win32::CreateNamedPipeW(
				m_pipeName.c_str(),             // pipe name
				m_openMode,
				m_pipeMode,
				m_maxInstances,                 // max. instances  
				m_size,                         // output buffer size 
				m_size,                         // input buffer size 
				0,                              // client time-out 
				!m_sid.empty() ? &sa : nullptr
			);
			if (not m_sid.empty())
				Win32::LocalFree(sa.lpSecurityDescriptor);
			if (not m_pipe)
				throw Error::Win32Error(Win32::GetLastError(), "Failed to create named pipe");
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

		virtual bool InternalUnreadCharactersRemaining(Win32::DWORD& charactersRemaining, const bool throwOnError) const
		{
			if (not m_pipe)
				return false;
			charactersRemaining = 0;
			bool succeeded = Win32::PeekNamedPipe(
				m_pipe.GetHandle(),
				nullptr,
				0,
				nullptr,
				nullptr,
				&charactersRemaining
			);
			if (not succeeded)
			{
				if (throwOnError)
					throw Error::Win32Error(Win32::GetLastError(), "PeekNamedPipe() failed");
				return false;
			}

			charactersRemaining /= sizeof(wchar_t);
			return true;
		}

		protected:
		RAII::Win32Handle m_pipe;
		std::wstring m_pipeName;
		std::wstring m_sid;
		Win32::DWORD m_size = 0;
		Win32::DWORD m_maxInstances = 0;
		bool m_isInheritable = false;
		Win32::DWORD m_pipeMode = 0;
		Win32::DWORD m_openMode = 0;
	};
}
