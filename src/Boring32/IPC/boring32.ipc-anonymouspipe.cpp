module boring32.ipc:anonymouspipe;
import boring32.strings;
import boring32.error;
import <stdexcept>;
import <iostream>;

namespace Boring32::IPC
{
	AnonymousPipe::~AnonymousPipe() = default;
	AnonymousPipe::AnonymousPipe() = default;
	AnonymousPipe::AnonymousPipe(const AnonymousPipe& other) = default;
	AnonymousPipe::AnonymousPipe(AnonymousPipe&& other) noexcept = default;
	AnonymousPipe& AnonymousPipe::operator=(const AnonymousPipe& other) = default;
	AnonymousPipe& AnonymousPipe::operator=(AnonymousPipe&& other) noexcept = default;

	AnonymousPipe::AnonymousPipe(
		const bool inheritable, 
		const DWORD size
	) :	m_size(size)
	{
		SECURITY_ATTRIBUTES secAttrs{ 0 };
		secAttrs.nLength = sizeof(secAttrs);
		secAttrs.bInheritHandle = inheritable;
		bool succeeded = CreatePipe(&m_readHandle, &m_writeHandle, &secAttrs, size);
		if (succeeded == false)
			throw std::runtime_error("Failed to create anonymous pipe");
	}

	AnonymousPipe::AnonymousPipe(
		const DWORD size,
		const HANDLE readHandle,
		const HANDLE writeHandle
	)
	:	m_size(size),
		m_readHandle(readHandle),
		m_writeHandle(writeHandle)
	{ }

	void AnonymousPipe::Close()
	{
		m_readHandle.Close();
		m_writeHandle.Close();
	}

	void AnonymousPipe::Write(const std::wstring& msg)
	{
		if (!m_writeHandle)
			throw Error::Boring32Error("No active write handle.");

		if ((GetUsedSize() + msg.size()) >= m_size)
			throw Error::Boring32Error("Pipe cannot fit message");

		DWORD bytesWritten = 0;
		const bool success = WriteFile(
			m_writeHandle.GetHandle(),
			msg.data(),
			static_cast<DWORD>(msg.size()*sizeof(wchar_t)),
			&bytesWritten,
			nullptr
		);
		if (!success)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("Write operation failed.", lastError);
		}
	}

	std::wstring AnonymousPipe::Read()
	{
		if (m_readHandle == nullptr)
			throw std::runtime_error("No active read handle.");

		std::wstring msg;
		DWORD bytesRead = 0;
		msg.resize(m_size);
		bool success = ReadFile(
			m_readHandle.GetHandle(),
			&msg[0],
			(DWORD)(msg.size() * sizeof(wchar_t)),
			&bytesRead,
			nullptr
		);
		if (success == false)
			throw std::runtime_error("Read operation failed");

		msg.erase(std::find(msg.begin(), msg.end(), '\0'), msg.end());

		return msg;
	}

	void AnonymousPipe::SetMode(const DWORD mode)
	{
		if(m_readHandle == nullptr && m_writeHandle == nullptr)
			throw std::runtime_error("Cannot set pipe mode on null pipes");

		HANDLE handleToSet = nullptr;
		if (m_readHandle != nullptr)
			handleToSet = m_readHandle.GetHandle();
		else if (m_writeHandle != nullptr)
			handleToSet = m_writeHandle.GetHandle();
		if (handleToSet == nullptr)
			throw std::runtime_error("No handleToSet");

		// Do not pass PIPE_READMODE_MESSAGE, as anonymous pipes are created in
		// byte mode, and cannot be changed.
		bool succeeded = SetNamedPipeHandleState(handleToSet, &m_mode, nullptr, nullptr);
		if (succeeded == false)
			throw std::runtime_error("Failed to create set pipe handle state");
	}

	void AnonymousPipe::CloseRead()
	{
		m_readHandle.Close();
	}

	void AnonymousPipe::CloseWrite()
	{
		m_writeHandle.Close();
	}

	HANDLE AnonymousPipe::GetRead() const noexcept
	{
		return m_readHandle.GetHandle();
	}

	HANDLE AnonymousPipe::GetWrite() const noexcept
	{
		return m_writeHandle.GetHandle();
	}

	DWORD AnonymousPipe::GetSize() const
	{
		return m_size;
	}

	DWORD AnonymousPipe::GetUsedSize() const
	{
		DWORD charactersInPipe = 0;
		// We do this sequence of actions to determine how much space
		// is used in the passed pipe handles, if any.
		HANDLE handleToDetermineBytesAvailable = nullptr;
		if (m_readHandle != nullptr)
			handleToDetermineBytesAvailable = m_readHandle.GetHandle();
		else if (m_writeHandle != nullptr)
			handleToDetermineBytesAvailable = m_writeHandle.GetHandle();
		if (handleToDetermineBytesAvailable)
		{
			bool success = PeekNamedPipe(
				m_readHandle.GetHandle(),
				nullptr,
				0,
				nullptr,
				&charactersInPipe,
				nullptr
			);
			if (!success)
			{
				const auto lastError = GetLastError();
				throw Error::Win32Error("PeekNamedPipe() failed", lastError);
			}
			charactersInPipe /= sizeof(wchar_t);
		}

		return charactersInPipe;
	}

	DWORD AnonymousPipe::GetRemainingSize() const
	{
		return m_size - GetUsedSize();
	}
}