module boring32.winsock:socket;
import :winsockerror;

namespace Boring32::WinSock
{
	Socket::~Socket()
	{
		Close();
	}

	Socket::Socket(
		const int addressFamily,
		const int type,
		const int protocol
	)
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-socket
		m_socket = socket(
			addressFamily,
			type,
			protocol
		);
		if (m_socket == INVALID_SOCKET)
		{
			const auto lastError = WSAGetLastError();
			Error::ThrowNested(
				Error::Win32Error("socket() failed", lastError, L"ws2_32.dll"),
				WinSockError("Failed to open socket")
			);
		}
	}

	Socket::Socket(Socket&& other) noexcept
	{
		Move(other);
	}

	Socket::Socket(const SOCKET socket)
	{
		m_socket = socket;
	}

	void Socket::Close()
	{
		if (m_socket && m_socket != INVALID_SOCKET)
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
		}
	}
	
	SOCKET Socket::GetHandle() const noexcept
	{
		return m_socket;
	}

	Socket& Socket::operator=(Socket&& other) noexcept
	{
		return Move(other);
	}

	Socket& Socket::Move(Socket& other)
	{
		Close();
		m_socket = other.m_socket;
		other.m_socket = INVALID_SOCKET;
		return *this;
	}

	Socket::operator SOCKET() const noexcept
	{
		return m_socket;
	}

	SOCKET* Socket::operator& () noexcept
	{
		return &m_socket;
	}

	SOCKET Socket::operator*() noexcept
	{
		return m_socket;
	}
}
