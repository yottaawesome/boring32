export module boring32:winsock.socket;
import std;
import :win32;
import :winsock.winsockerror;

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
		Socket& operator=(const Socket&) = delete;

		Socket(Socket&& other) noexcept
		{
			Move(other);
		}
		auto operator=(Socket&& other) noexcept -> Socket&
		{
			return Move(other);
		}

		Socket(Win32::WinSock::SOCKET socket)
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
			if (m_socket == Win32::WinSock::InvalidSocket)
			{
				auto lastError = Win32::WinSock::WSAGetLastError();
				Error::ThrowNested(
					Error::Win32Error{static_cast<Win32::DWORD>(lastError), "socket() failed", L"ws2_32.dll"},
					WinSockError("Failed to open socket")
				);
			}
		}

		operator Win32::WinSock::SOCKET() const noexcept
		{
			return m_socket;
		}

		auto operator&() noexcept -> Win32::WinSock::SOCKET*
		{
			return &m_socket;
		}

		auto operator*() noexcept -> Win32::WinSock::SOCKET
		{
			return m_socket;
		}

		void Close()
		{
			if (m_socket and m_socket != Win32::WinSock::InvalidSocket)
			{
				Win32::WinSock::closesocket(m_socket);
				m_socket = Win32::WinSock::InvalidSocket;
			}
		}

		auto GetHandle() const noexcept -> Win32::WinSock::SOCKET
		{
			return m_socket;
		}

	private:
		auto Move(Socket& other) noexcept -> Socket&
		{
			Close();
			m_socket = std::exchange(other.m_socket, Win32::WinSock::InvalidSocket);
			return *this;
		}

		Win32::WinSock::SOCKET m_socket = Win32::WinSock::InvalidSocket;
	};
}
