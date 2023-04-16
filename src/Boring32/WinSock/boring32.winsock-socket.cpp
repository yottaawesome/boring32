module boring32.winsock:socket;

namespace Boring32::WinSock
{
	Socket::~Socket()
	{
		Close();
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
}
