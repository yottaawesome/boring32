export module boring32:ipc.anonymouspipe;
import std;
import :win32;
import :raii;
import :strings;
import :error;

export namespace Boring32::IPC
{
	class AnonymousPipe
	{
	public:
		AnonymousPipe() = default;
		AnonymousPipe(const AnonymousPipe& other) = default;
		AnonymousPipe(AnonymousPipe&& other) noexcept = default;
		auto operator=(const AnonymousPipe& other) -> AnonymousPipe& = default;
		auto operator=(AnonymousPipe&& other) noexcept -> AnonymousPipe& = default;

		AnonymousPipe(bool inheritable, Win32::DWORD size) 
			: m_size(size)
		{
			auto secAttrs = Win32::SECURITY_ATTRIBUTES{
				.nLength = sizeof(Win32::SECURITY_ATTRIBUTES),
				.bInheritHandle = inheritable
			};
			auto success = Win32::CreatePipe(
				&m_readHandle,
				&m_writeHandle,
				&secAttrs,
				size
			);
			if (not success)
				throw Error::Win32Error{Win32::GetLastError(), "Failed creating anonymous pipe: CreatePipe() failed."};
		}

		AnonymousPipe(Win32::DWORD size, Win32::HANDLE readHandle, Win32::HANDLE writeHandle) 
			: m_size(size),
			m_readHandle(readHandle),
			m_writeHandle(writeHandle)
		{ }

		void Write(this auto&& self, const std::wstring& msg)
		{
			if (not m_writeHandle)
				throw Error::Boring32Error{"No active write handle."};

			if ((self.GetUsedSize() + msg.size()) >= self.m_size)
				throw Error::Boring32Error{"Pipe cannot fit message"};

			auto bytesWritten = Win32::DWORD{};
			auto success = 
				Win32::WriteFile(
					self.m_writeHandle.GetHandle(),
					msg.data(),
					static_cast<Win32::DWORD>(msg.size() * sizeof(wchar_t)),
					&bytesWritten,
					nullptr
				);
			if (not success)
				throw Error::Win32Error{Win32::GetLastError(), "Write operation failed."};
		}

		auto Read(this auto&& self) -> std::wstring
		{
			if (not self.m_readHandle)
				throw Error::Boring32Error{"No active read handle."};

			auto msg = std::wstring{};
			auto bytesRead = Win32::DWORD{};
			msg.resize(self.m_size);
			auto success = Win32::ReadFile(
				self.m_readHandle.GetHandle(),
				&msg[0],
				static_cast<Win32::DWORD>(msg.size() * sizeof(wchar_t)),
				&bytesRead,
				nullptr
			);
			if (not success)
				throw Error::Win32Error{Win32::GetLastError(), "ReadFile() failed"};
			std::erase_if(msg, [](wchar_t c) { return c == '\0'; });
			return msg;
		}

		void CloseRead(this auto&& self)
		{
			self.m_readHandle.Close();
		}

		void CloseWrite(this auto&& self)
		{
			self.m_writeHandle.Close();
		}

		void SetMode(this auto&& self, Win32::DWORD mode)
		{
			if (not self.m_readHandle && not self.m_writeHandle)
				throw Error::Boring32Error{"Cannot set pipe mode on null pipes"};

			auto handleToSet = Win32::HANDLE{};
			if (self.m_readHandle)
				handleToSet = self.m_readHandle.GetHandle();
			else if (self.m_writeHandle)
				handleToSet = self.m_writeHandle.GetHandle();
			if (not handleToSet)
				throw Error::Boring32Error{ "No handleToSet" };

			// Do not pass PIPE_READMODE_MESSAGE, as anonymous pipes are created in
			// byte mode, and cannot be changed.
			// https://learn.microsoft.com/en-us/windows/win32/api/namedpipeapi/nf-namedpipeapi-setnamedpipehandlestate
			auto succeeded = 
				Win32::SetNamedPipeHandleState(
					handleToSet,
					&self.m_mode,
					nullptr,
					nullptr
				);
			if (not succeeded)
				throw Error::Win32Error{Win32::GetLastError(), "SetNamedPipeHandleState() failed."};
		}

		auto GetRead(this auto&& self) noexcept -> Win32::HANDLE
		{
			return self.m_readHandle.GetHandle();
		}

		auto GetWrite(this auto&& self) noexcept -> Win32::HANDLE
		{
			return self.m_writeHandle.GetHandle();
		}

		auto GetSize(this auto&& self) noexcept -> Win32::DWORD
		{
			return self.m_size;
		}

		auto GetUsedSize(this auto&& self) -> Win32::DWORD
		{
			auto charactersInPipe = Win32::DWORD{};
			// We do this sequence of actions to determine how much space
			// is used in the passed pipe handles, if any.
			auto handleToDetermineBytesAvailable = Win32::HANDLE{};
			if (self.m_readHandle)
				handleToDetermineBytesAvailable = self.m_readHandle.GetHandle();
			else if (self.m_writeHandle)
				handleToDetermineBytesAvailable = self.m_writeHandle.GetHandle();

			if (handleToDetermineBytesAvailable)
			{
				auto success = 
					Win32::PeekNamedPipe(
						self.m_readHandle.GetHandle(),
						nullptr,
						0,
						nullptr,
						&charactersInPipe,
						nullptr
					);
				if (not success)
					throw Error::Win32Error{Win32::GetLastError(), "PeekNamedPipe() failed"};
				charactersInPipe /= sizeof(wchar_t);
			}

			return charactersInPipe;
		}

		auto GetRemainingSize(this auto&& self) noexcept -> Win32::DWORD
		{
			return self.m_size - self.GetUsedSize();
		}

		void Close(this auto&& self)
		{
			self.m_readHandle.Close();
			self.m_writeHandle.Close();
		}

	protected:
		Win32::DWORD m_size = 0;
		Win32::DWORD m_mode = 0;
		RAII::SharedHandle m_readHandle;
		RAII::SharedHandle m_writeHandle;
	};
}