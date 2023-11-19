export module boring32.winsock:socket;
import boring32.win32;
import :winsockerror;

export namespace Boring32::WinSock
{
	class Socket final
	{
		public:
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

			Socket(
				const int addressFamily,
				const int type,
				const int protocol
			)
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
						Error::Win32Error("socket() failed", lastError, L"ws2_32.dll"),
						WinSockError("Failed to open socket")
					);
				}
			}

		public:
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

		public:
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

		private:
			Win32::WinSock::SOCKET m_socket = Win32::WinSock::_INVALID_SOCKET;
	};
}
