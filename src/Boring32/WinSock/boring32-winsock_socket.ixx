export module boring32:winsock_socket;
import boring32.shared;
import :winsock_winsockerror;

export namespace Boring32::WinSock
{
	struct Socket final
	{
		~Socket()
		{
			Close();
		}

		Socket() = default;

		Socket(const Socket&) = delete;

		Socket(Socket&& other) noexcept
		{
			Move(other);
		}

		Socket(const Win32::WinSock::SOCKET socket)
		{
			m_socket = socket;
		}

		Socket(int addressFamily, int type, int protocol)
		{
			// https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-socket
			m_socket = Win32::WinSock::socket(
				addressFamily,
				type,
				protocol
			);
			if (m_socket == Win32::WinSock::_INVALID_SOCKET)
			{
				const auto lastError = Win32::WinSock::WSAGetLastError();
				Error::ThrowNested(
					Error::Win32Error(lastError, "socket() failed", L"ws2_32.dll"),
					WinSockError("Failed to open socket")
				);
			}
		}

		Socket& operator=(Socket&& other) noexcept
		{
			return Move(other);
		}

		Socket& operator=(const Socket&) = delete;
		operator Win32::WinSock::SOCKET() const noexcept
		{
			return m_socket;
		}

		Win32::WinSock::SOCKET* operator&() noexcept
		{
			return &m_socket;
		}

		Win32::WinSock::SOCKET operator*() noexcept
		{
			return m_socket;
		}

		void Close()
		{
			if (m_socket && m_socket != Win32::WinSock::_INVALID_SOCKET)
			{
				Win32::WinSock::closesocket(m_socket);
				m_socket = Win32::WinSock::_INVALID_SOCKET;
			}
		}

		Win32::WinSock::SOCKET GetHandle() const noexcept
		{
			return m_socket;
		}

		private:
		Socket& Move(Socket& other)
		{
			Close();
			m_socket = other.m_socket;
			other.m_socket = Win32::WinSock::_INVALID_SOCKET;
			return *this;
		}

		Win32::WinSock::SOCKET m_socket = Win32::WinSock::_INVALID_SOCKET;
	};
}
