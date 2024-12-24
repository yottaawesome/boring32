export module boring32:ipc_anonymouspipe;
import boring32.shared;
import :raii;
import :strings;
import :error;

export namespace Boring32::IPC
{
	struct AnonymousPipe
	{
		virtual ~AnonymousPipe() = default;
		AnonymousPipe() = default;
		AnonymousPipe(const AnonymousPipe& other) = default;
		AnonymousPipe(AnonymousPipe&& other) noexcept = default;
		virtual AnonymousPipe& operator=(const AnonymousPipe& other) = default;
		virtual AnonymousPipe& operator=(AnonymousPipe&& other) noexcept = default;

		AnonymousPipe(bool inheritable, Win32::DWORD size) 
			: m_size(size)
		{
			Win32::SECURITY_ATTRIBUTES secAttrs{
				.nLength = sizeof(Win32::SECURITY_ATTRIBUTES),
				.bInheritHandle = inheritable
			};
			bool success = Win32::CreatePipe(
				&m_readHandle,
				&m_writeHandle,
				&secAttrs,
				size
			);
			if (not success)
				throw Error::Win32Error(Win32::GetLastError(), "Failed creating anonymous pipe: CreatePipe() failed.");
		}

		AnonymousPipe(Win32::DWORD size, Win32::HANDLE readHandle, Win32::HANDLE writeHandle) 
			: m_size(size),
			m_readHandle(readHandle),
			m_writeHandle(writeHandle)
		{ }

		virtual void Write(const std::wstring& msg)
		{
			if (not m_writeHandle)
				throw Error::Boring32Error("No active write handle.");

			if ((GetUsedSize() + msg.size()) >= m_size)
				throw Error::Boring32Error("Pipe cannot fit message");

			Win32::DWORD bytesWritten = 0;
			bool success = Win32::WriteFile(
				m_writeHandle.GetHandle(),
				msg.data(),
				static_cast<Win32::DWORD>(msg.size() * sizeof(wchar_t)),
				&bytesWritten,
				nullptr
			);
			if (not success)
				throw Error::Win32Error(Win32::GetLastError(), "Write operation failed.");
		}

		virtual std::wstring Read()
		{
			if (not m_readHandle)
				throw Error::Boring32Error("No active read handle.");

			std::wstring msg;
			Win32::DWORD bytesRead = 0;
			msg.resize(m_size);
			bool success = Win32::ReadFile(
				m_readHandle.GetHandle(),
				&msg[0],
				static_cast<Win32::DWORD>(msg.size() * sizeof(wchar_t)),
				&bytesRead,
				nullptr
			);
			if (not success)
				throw Error::Win32Error(Win32::GetLastError(), "ReadFile() failed");

			std::erase_if(msg, [](wchar_t c) { return c == '\0'; });
			return msg;
		}

		virtual void CloseRead()
		{
			m_readHandle.Close();
		}

		virtual void CloseWrite()
		{
			m_writeHandle.Close();
		}

		virtual void SetMode(const Win32::DWORD mode)
		{
			if (not m_readHandle && not m_writeHandle)
				throw Error::Boring32Error("Cannot set pipe mode on null pipes");

			Win32::HANDLE handleToSet = nullptr;
			if (m_readHandle)
				handleToSet = m_readHandle.GetHandle();
			else if (m_writeHandle)
				handleToSet = m_writeHandle.GetHandle();
			if (not handleToSet)
				throw Error::Boring32Error("No handleToSet");

			// Do not pass PIPE_READMODE_MESSAGE, as anonymous pipes are created in
			// byte mode, and cannot be changed.
			// https://learn.microsoft.com/en-us/windows/win32/api/namedpipeapi/nf-namedpipeapi-setnamedpipehandlestate
			bool succeeded = Win32::SetNamedPipeHandleState(
				handleToSet,
				&m_mode,
				nullptr,
				nullptr
			);
			if (not succeeded)
				throw Error::Win32Error(Win32::GetLastError(), "SetNamedPipeHandleState() failed.");
		}

		virtual Win32::HANDLE GetRead() const noexcept
		{
			return m_readHandle.GetHandle();
		}

		virtual Win32::HANDLE GetWrite() const noexcept
		{
			return m_writeHandle.GetHandle();
		}

		virtual Win32::DWORD GetSize() const noexcept
		{
			return m_size;
		}

		virtual Win32::DWORD GetUsedSize() const
		{
			Win32::DWORD charactersInPipe = 0;
			// We do this sequence of actions to determine how much space
			// is used in the passed pipe handles, if any.
			Win32::HANDLE handleToDetermineBytesAvailable = nullptr;
			if (m_readHandle)
				handleToDetermineBytesAvailable = m_readHandle.GetHandle();
			else if (m_writeHandle)
				handleToDetermineBytesAvailable = m_writeHandle.GetHandle();

			if (handleToDetermineBytesAvailable)
			{
				bool success = Win32::PeekNamedPipe(
					m_readHandle.GetHandle(),
					nullptr,
					0,
					nullptr,
					&charactersInPipe,
					nullptr
				);
				if (not success)
					throw Error::Win32Error(Win32::GetLastError(), "PeekNamedPipe() failed");
				charactersInPipe /= sizeof(wchar_t);
			}

			return charactersInPipe;
		}

		virtual Win32::DWORD GetRemainingSize() const
		{
			return m_size - GetUsedSize();
		}

		virtual void Close()
		{
			m_readHandle.Close();
			m_writeHandle.Close();
		}

		protected:
		Win32::DWORD m_size = 0;
		Win32::DWORD m_mode = 0;
		RAII::Win32Handle m_readHandle;
		RAII::Win32Handle m_writeHandle;
	};
}