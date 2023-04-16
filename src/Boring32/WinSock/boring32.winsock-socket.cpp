module boring32.winsock:socket;

namespace Boring32::WinSock
{
	Socket::~Socket()
	{
		Close();
	}

	Socket::Socket(Socket&& other) noexcept
	{
		m_socket = other.m_socket;
		other.m_socket = INVALID_SOCKET;
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
}
