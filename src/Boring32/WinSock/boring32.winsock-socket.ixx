module;

#include <source_location>;

export module boring32.winsock:socket;
import "win32.hpp";
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

			Socket(const SOCKET socket)
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

		public:
			Socket& operator=(Socket&& other) noexcept
			{
				return Move(other);
			}

			Socket& operator=(const Socket&) = delete;
			operator SOCKET() const noexcept
			{
				return m_socket;
			}

			SOCKET* operator&() noexcept
			{
				return &m_socket;
			}

			SOCKET operator*() noexcept
			{
				return m_socket;
			}

		public:
			void Close()
			{
				if (m_socket && m_socket != INVALID_SOCKET)
				{
					closesocket(m_socket);
					m_socket = INVALID_SOCKET;
				}
			}

			SOCKET GetHandle() const noexcept
			{
				return m_socket;
			}

		private:
			Socket& Move(Socket& other)
			{
				Close();
				m_socket = other.m_socket;
				other.m_socket = INVALID_SOCKET;
				return *this;
			}

		private:
			SOCKET m_socket = INVALID_SOCKET;
	};
}
