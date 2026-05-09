export module boring32:ipc.namedpipeserverbase;
import std;
import :win32;
import :error;
import :raii;

export namespace Boring32::IPC
{
	class NamedPipeServerBase
	{
	public:
		// example of DACL/SID strings: "D:(A;;GA;;;BA)(A;;GR;;;BU)" and "D:(A;;GRGW;;;BU)"
		~NamedPipeServerBase()
		{
			Close();
		}

		NamedPipeServerBase() = default;
			
		NamedPipeServerBase(const NamedPipeServerBase& other)
			: NamedPipeServerBase()
		{
			Copy(other);
		}
		auto operator=(const NamedPipeServerBase& other) -> NamedPipeServerBase&
		{
			Copy(other);
			return *this;
		}

		NamedPipeServerBase(NamedPipeServerBase&& other) noexcept
			: NamedPipeServerBase()
		{
			Move(other);
		}
		auto operator=(NamedPipeServerBase&& other) noexcept -> NamedPipeServerBase&
		{
			Move(other);
			return *this;
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

		void Close(this auto&& self)
		{
			self.Disconnect();
			self.m_pipe.Close();
		}

		void Disconnect(this auto&& self)
		{
			if (self.m_pipe)
				Win32::DisconnectNamedPipe(self.m_pipe.GetHandle());
		}

		void Flush(this auto&& self)
		{
			if (not self.m_pipe)
				throw Error::Boring32Error("No pipe to flush");
			if (not Win32::FlushFileBuffers(self.m_pipe.GetHandle()))
				throw Error::Win32Error{Win32::GetLastError(), "Flush() failed"};
		}

		auto GetInternalHandle() -> RAII::SharedHandle&
		{
			return m_pipe;
		}

		auto GetName() const -> std::wstring
		{
			return m_pipeName;
		}

		auto GetSize() const -> Win32::DWORD
		{
			return m_size;
		}

		auto GetMaxInstances() const -> Win32::DWORD
		{
			return m_maxInstances;
		}

		auto GetPipeMode() const -> Win32::DWORD
		{
			return m_pipeMode;
		}

		auto GetOpenMode() const -> Win32::DWORD
		{
			return m_openMode;
		}

		auto UnreadCharactersRemaining(this auto&& self) -> Win32::DWORD
		{
			auto charactersRemaining = Win32::DWORD{};
			self.InternalUnreadCharactersRemaining(charactersRemaining, true);
			return charactersRemaining;
		}

		auto TryUnreadCharactersRemaining(
			Win32::DWORD& charactersRemaining
		) noexcept -> bool
		{
			return InternalUnreadCharactersRemaining(charactersRemaining, false);
		}

		void CancelCurrentThreadIo(this auto&& self)
		{
			if (not self.m_pipe)
				throw Error::Boring32Error("pipe is nullptr");
			if (not Win32::CancelIo(self.m_pipe.GetHandle()))
				throw Error::Win32Error{Win32::GetLastError(), "CancelIo() failed"};
		}

		auto TryCancelCurrentThreadIo(this auto&& self) noexcept -> bool
		try
		{
			self.CancelCurrentThreadIo();
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}

		auto CancelCurrentProcessIo(this auto&& self, Win32::OVERLAPPED* overlapped)
		{
			if (not self.m_pipe)
				throw Error::Boring32Error("pipe is nullptr");
			if (not Win32::CancelIoEx(self.m_pipe.GetHandle(), overlapped))
				throw Error::Win32Error{Win32::GetLastError(), "CancelIo() failed"};
		}

		auto TryCancelCurrentProcessIo(this auto&& self, Win32::OVERLAPPED* overlapped) noexcept -> bool
		try
		{
			self.CancelCurrentProcessIo(overlapped);
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}

	protected:
		void InternalCreatePipe(this auto&& self)
		{
			if (not self.m_pipeName.starts_with(L"\\\\.\\pipe\\"))
				self.m_pipeName = L"\\\\.\\pipe\\" + self.m_pipeName;

			auto sa = Win32::SECURITY_ATTRIBUTES{
				.nLength = sizeof(Win32::SECURITY_ATTRIBUTES),
				.bInheritHandle = self.m_isInheritable
			};
			if (not self.m_sid.empty())
			{
				auto converted = 
					Win32::ConvertStringSecurityDescriptorToSecurityDescriptorW(
						self.m_sid.c_str(),
						Win32::SddlRevision1,
						&sa.lpSecurityDescriptor,
						nullptr
					);
				if (not converted)
					throw Error::Win32Error{Win32::GetLastError(), "Failed to convert security descriptor"};
			}

			// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-createnamedpipea
			self.m_pipe = Win32::CreateNamedPipeW(
				self.m_pipeName.c_str(),             // pipe name
				self.m_openMode,
				self.m_pipeMode,
				self.m_maxInstances,                 // max. instances  
				self.m_size,                         // output buffer size 
				self. m_size,                         // input buffer size 
				0,                              // client time-out 
				!self.m_sid.empty() ? &sa : nullptr
			);
			if (not self.m_sid.empty())
				Win32::LocalFree(sa.lpSecurityDescriptor);
			if (not self.m_pipe)
				throw Error::Win32Error{Win32::GetLastError(), "Failed to create named pipe"};
		}

		void Copy(const NamedPipeServerBase& other)
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

		void Move(NamedPipeServerBase& other) noexcept
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

		auto InternalUnreadCharactersRemaining(
			this auto&& self,
			Win32::DWORD& charactersRemaining, 
			const bool throwOnError
		) -> bool
		{
			if (not self.m_pipe)
				return false;
			charactersRemaining = 0;
			auto succeeded = Win32::PeekNamedPipe(
				self.m_pipe.GetHandle(),
				nullptr,
				0,
				nullptr,
				nullptr,
				&charactersRemaining
			);
			if (not succeeded)
			{
				if (throwOnError)
					throw Error::Win32Error{Win32::GetLastError(), "PeekNamedPipe() failed"};
				return false;
			}

			charactersRemaining /= sizeof(wchar_t);
			return true;
		}

	protected:
		RAII::SharedHandle m_pipe;
		std::wstring m_pipeName;
		std::wstring m_sid;
		Win32::DWORD m_size = 0;
		Win32::DWORD m_maxInstances = 0;
		bool m_isInheritable = false;
		Win32::DWORD m_pipeMode = 0;
		Win32::DWORD m_openMode = 0;
	};
}
