export module boring32:winsock.tcpsocket;
import std;
import :win32;
import :error;
import :strings;
import :winsock.uniqueptrs;
import :winsock.winsockerror;
import :winsock.resolvedname;

export namespace Boring32::WinSock
{
	// Based on https://docs.microsoft.com/en-us/windows/win32/winsock/winsock-client-application
	class TCPSocket final
	{
	public:
		static constexpr Win32::WinSock::SOCKET InvalidSocket = Win32::WinSock::InvalidSocket;

		~TCPSocket()
		{
			Close();
		}

		TCPSocket() = default;

		TCPSocket(const TCPSocket& other) = delete;

		TCPSocket(TCPSocket&& other) noexcept
		{
			Move(other);
		}

		TCPSocket(std::wstring host, unsigned portNumber)
			: m_host(std::move(host)),
			m_portNumber(portNumber)
		{ 
			m_socket = Win32::WinSock::socket(
				Win32::WinSock::AddressFamily::IPv4,
				Win32::WinSock::_SOCK_STREAM,
				Win32::WinSock::_IPPROTO_TCP
			);
			if (m_socket == Win32::WinSock::InvalidSocket)
			{
				auto lastError = Win32::WinSock::WSAGetLastError();
				Error::ThrowNested(
					Error::Win32Error(lastError, "socket() failed", L"ws2_32.dll"),
					WinSockError("Failed to open socket")
				);
			}
		}

		TCPSocket& operator=(const TCPSocket& other) = delete;
		TCPSocket& operator=(TCPSocket&& other) noexcept
		{
			return Move(other);
		}

		void Open()
		{
			auto name = ResolvedName{
				.Name = m_host,
				.Port = m_portNumber,
				.AddressType = Win32::WinSock::AddressFamily::IPv4,
				.Protocol = Win32::WinSock::IPPROTO::IPPROTO_TCP,
				.SocketType = Win32::WinSock::_SOCK_STREAM
			};
			m_addrPtr = std::move(name.AddrInfo);
			m_addressFamily = m_addrPtr->ai_family;
		}

		void Connect()
		{
			Connect(0, 0);
		}

		void Connect(Win32::DWORD socketTTL, Win32::DWORD maxRetryTimeout)
		{
			Open();
			if (socketTTL)
				SetSocketTTL(socketTTL);
			if (maxRetryTimeout)
				SetMaxRetryTimeout(maxRetryTimeout);

			// Trying to connect to a random socket on the remote host seems to trigger a long timeout
			// ending with error code 0X0000274C=10060=WSAETIMEDOUT. Can we adjust this timeout
			// and is it possible get a different error like WSAECONNREFUSED?
			// See https://social.msdn.microsoft.com/Forums/en-US/2202d113-212d-420d-9e7b-11268de9ce90/win32-tcp-connect-timeout
			// and https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
			// https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-connect
			auto connectionResult = Win32::WinSock::connect(
				m_socket,
				m_addrPtr->ai_addr,
				static_cast<int>(m_addrPtr->ai_addrlen)
			);
			if (connectionResult == Win32::WinSock::_SOCKET_ERROR)
			{
				Error::ThrowNested(
					Error::Win32Error{static_cast<Win32::DWORD>(Win32::WinSock::WSAGetLastError()), "connect() failed", L"ws2_32.dll"},
					WinSockError("Failed to connect socket")
				);
			}
		}

		void Close()
		{
			if (not m_socket)
				return;
			if (m_socket == InvalidSocket)
				return;

			Win32::WinSock::closesocket(m_socket);
			m_socket = InvalidSocket;
		}

		void Send(const std::vector<std::byte>& data)
		{
			if (not m_socket or m_socket == InvalidSocket)
				throw Error::Boring32Error{ "Socket is not valid" };
			if (data.empty())
				return;

			auto totalSentBytes = size_t{};
			auto totalBytesToSend = data.size();
			while (totalSentBytes < totalBytesToSend)
			{
				const char* bufferStart = reinterpret_cast<const char*>(&data[totalSentBytes]);
				auto remainingBytes = size_t{ totalBytesToSend - totalSentBytes };

				// https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-send
				auto sentBytes = Win32::WinSock::send(
					m_socket,
					bufferStart,
					static_cast<int>(remainingBytes),
					0
				);
				if (sentBytes == Win32::WinSock::_SOCKET_ERROR)
				{
					Error::ThrowNested(
						Error::Win32Error{static_cast<Win32::DWORD>(Win32::WinSock::WSAGetLastError()), "send() failed", L"ws2_32.dll"},
						WinSockError("Failed to send data through socket")
					);
				}
				totalSentBytes += sentBytes;
			}
		}

		auto Receive(unsigned bytesToRead) -> std::vector<std::byte>
		{
			if (not m_socket or m_socket == InvalidSocket)
				throw WinSockError{ "Socket is not valid" };
			if (not bytesToRead)
				return {};

			// https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-recv
			auto recvbuf = std::vector<std::byte>(bytesToRead);
			auto actualBytesRead = Win32::WinSock::recv(m_socket, reinterpret_cast<char*>(&recvbuf[0]), bytesToRead, 0);
			if (actualBytesRead < 0)
			{
				Error::ThrowNested(
					Error::Win32Error{static_cast<Win32::DWORD>(Win32::WinSock::WSAGetLastError()), "recv() failed", L"ws2_32.dll"},
					WinSockError("Failed to receive data through socket")
				);
			}

			recvbuf.resize(actualBytesRead);
			return recvbuf;
		}

		void SetSocketTTL(const Win32::DWORD ttl)
		{
			if (not m_socket or m_socket == InvalidSocket)
				throw WinSockError{ "Not in a valid state to set TTL support" };

			auto layer = Win32::DWORD{};
			auto argument = Win32::DWORD{};
			switch (m_addressFamily)
			{
				case Win32::WinSock::AddressFamily::IPv4:
					layer = Win32::WinSock::_IPPROTO_IP;
					argument = Win32::WinSock::_IP_TTL;
					break;

				case Win32::WinSock::AddressFamily::IPv6:
					layer = Win32::WinSock::IPPROTO::IPPROTO_IPV6;
					argument = Win32::WinSock::_IPV6_UNICAST_HOPS;
					break;

				default:
					throw WinSockError{ "Unknown address family" };
			}

			// Query support for the argument
			auto optVal = Win32::DWORD{};
			int optLen = sizeof(optVal);
			int optResult = Win32::WinSock::getsockopt(
				m_socket,
				layer,
				argument,
				reinterpret_cast<char*>(&optVal),
				&optLen
			);
			if (optResult == Win32::WinSock::_SOCKET_ERROR)
			{
				Error::ThrowNested(
					Error::Win32Error{static_cast<Win32::DWORD>(Win32::WinSock::WSAGetLastError()), "getsockopt() failed", L"ws2_32.dll"},
					WinSockError{ "TTL option is not supported" }
				);
			}

			// Actually set the argument
			optVal = ttl;
			optResult = Win32::WinSock::setsockopt(
				m_socket,
				layer,
				argument,
				reinterpret_cast<char*>(&optVal),
				optLen
			);
			if (optResult == Win32::WinSock::_SOCKET_ERROR)
			{
				Error::ThrowNested(
					Error::Win32Error{static_cast<Win32::DWORD>(Win32::WinSock::WSAGetLastError()), "setsockopt() failed", L"ws2_32.dll"},
					WinSockError{ "Failed to set option" }
				);
			}
		}

		void SetMaxRetryTimeout(Win32::DWORD timeoutSeconds)
		{
			if (not m_socket or m_socket == InvalidSocket)
				throw WinSockError{ "Not in a valid state to set TTL support" };

			// Query support for the argument
			auto optVal = Win32::DWORD{};
			auto optLen = sizeof(optVal);
			auto optResult = getsockopt(
				m_socket,
				Win32::WinSock::IPPROTO::IPPROTO_TCP,
				Win32::WinSock::_TCP_MAXRT,
				reinterpret_cast<char*>(&optVal),
				reinterpret_cast<int*>(&optLen)
			);
			if (optResult == Win32::WinSock::_SOCKET_ERROR)
				Error::ThrowNested(
					Error::Win32Error{static_cast<Win32::DWORD>(Win32::WinSock::WSAGetLastError()), "getsockopt() failed", L"ws2_32.dll"},
					WinSockError{ "RT option is not supported" }
				);

			optVal = timeoutSeconds;
			optResult = Win32::WinSock::setsockopt(
				m_socket,
				Win32::WinSock::IPPROTO::IPPROTO_TCP,
				Win32::WinSock::_TCP_MAXRT,
				reinterpret_cast<char*>(&optVal),
				static_cast<int>(optLen)
			);
			if (optResult == Win32::WinSock::_SOCKET_ERROR)
				Error::ThrowNested(
					Error::Win32Error{static_cast<Win32::DWORD>(Win32::WinSock::WSAGetLastError()), "setsockopt() failed", L"ws2_32.dll"},
					WinSockError{ "Failed to set RT option" }
				);
		}

		auto GetHost() const noexcept -> std::wstring
		{
			return m_host;
		}

		auto GetPort() const noexcept -> unsigned
		{
			return m_portNumber;
		}

		auto GetHandle() const noexcept -> Win32::WinSock::SOCKET
		{
			return m_socket;
		}

	private:
		auto Move(TCPSocket& other) noexcept -> TCPSocket&
		{
			Close();
			m_host = std::move(other.m_host);
			m_portNumber = other.m_portNumber;
			m_socket = other.m_socket;
			other.m_socket = InvalidSocket;
			m_addressFamily = other.m_addressFamily;
			return *this;
		}

		std::wstring m_host;
		unsigned m_portNumber = 0;
		Win32::WinSock::SOCKET m_socket = Win32::WinSock::InvalidSocket; // doesn't work with unique_ptr
		int m_addressFamily = 0;
		AddrInfoWUniquePtr m_addrPtr;
	};		
}
