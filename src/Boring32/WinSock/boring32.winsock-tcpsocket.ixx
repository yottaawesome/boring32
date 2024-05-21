export module boring32.winsock:tcpsocket;
import boring32.shared;
import boring32.error;
import boring32.strings;
import :uniqueptrs;
import :winsockerror;
import :resolvedname;

export namespace Boring32::WinSock
{
	// Based on https://docs.microsoft.com/en-us/windows/win32/winsock/winsock-client-application
	class TCPSocket final
	{
		public:
			static constexpr Win32::WinSock::SOCKET InvalidSocket = Win32::WinSock::_INVALID_SOCKET;

		public:
			~TCPSocket()
			{
				Close();
			}

			TCPSocket()
			{
			}

			TCPSocket(const TCPSocket& other) = delete;

			TCPSocket(TCPSocket&& other) noexcept
			{
				Move(other);
			}

			TCPSocket(const std::wstring host, const unsigned portNumber)
				: m_host(std::move(host)),
				m_portNumber(portNumber)
			{ 
				m_socket = Win32::WinSock::socket(
					Win32::WinSock::AddressFamily::IPv4,
					Win32::WinSock::_SOCK_STREAM,
					Win32::WinSock::_IPPROTO_TCP
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
			TCPSocket& operator=(const TCPSocket& other) = delete;
			TCPSocket& operator=(TCPSocket&& other) noexcept
			{
				return Move(other);
			}

		public:
			void Open()
			{


				ResolvedName name{ 
					m_host, 
					m_portNumber , 
					Win32::WinSock::AddressFamily::IPv4 , 
					Win32::WinSock::IPPROTO::IPPROTO_TCP, 
					Win32::WinSock::_SOCK_STREAM 
				};
				m_addrPtr = std::move(name.AddrInfo);
				m_addressFamily = m_addrPtr->ai_family;
			}

			void Connect()
			{
				Connect(0, 0);
			}

			void Connect(const Win32::DWORD socketTTL, const Win32::DWORD maxRetryTimeout)
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
				const int connectionResult = Win32::WinSock::connect(
					m_socket,
					m_addrPtr->ai_addr,
					static_cast<int>(m_addrPtr->ai_addrlen)
				);
				if (connectionResult == Win32::WinSock::_SOCKET_ERROR)
				{
					const auto lastError = Win32::WinSock::WSAGetLastError();
					Error::ThrowNested(
						Error::Win32Error("connect() failed", lastError, L"ws2_32.dll"),
						WinSockError("Failed to connect socket")
					);
				}
			}

			void Close()
			{
				if (!m_socket)
					return;
				if (m_socket == InvalidSocket)
					return;

				Win32::WinSock::closesocket(m_socket);
				m_socket = InvalidSocket;
			}

			void Send(const std::vector<std::byte>& data)
			{
				if (!m_socket || m_socket == InvalidSocket)
					throw Error::Boring32Error("Socket is not valid");
				if (data.empty())
					return;

				size_t totalSentBytes = 0;
				const size_t totalBytesToSend = data.size();
				while (totalSentBytes < totalBytesToSend)
				{
					const char* bufferStart = reinterpret_cast<const char*>(&data[totalSentBytes]);
					const size_t remainingBytes = totalBytesToSend - totalSentBytes;

					// https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-send
					const int sentBytes = Win32::WinSock::send(
						m_socket,
						bufferStart,
						static_cast<int>(remainingBytes),
						0
					);
					if (sentBytes == Win32::WinSock::_SOCKET_ERROR)
					{
						const auto lastError = Win32::WinSock::WSAGetLastError();
						Error::ThrowNested(
							Error::Win32Error("send() failed", lastError, L"ws2_32.dll"),
							WinSockError("Failed to send data through socket")
						);
					}
					totalSentBytes += sentBytes;
				}
			}

			std::vector<std::byte> Receive(const unsigned bytesToRead)
			{
				if (!m_socket || m_socket == InvalidSocket)
					throw WinSockError("Socket is not valid");
				if (!bytesToRead)
					return {};

				// https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-recv
				std::vector<std::byte> recvbuf(bytesToRead);
				const int actualBytesRead = Win32::WinSock::recv(m_socket, reinterpret_cast<char*>(&recvbuf[0]), bytesToRead, 0);
				if (actualBytesRead < 0)
				{
					const auto lastError = Win32::WinSock::WSAGetLastError();
					Error::ThrowNested(
						Error::Win32Error("recv() failed", lastError, L"ws2_32.dll"),
						WinSockError("Failed to receive data through socket")
					);
				}

				recvbuf.resize(actualBytesRead);
				return recvbuf;
			}

			void SetSocketTTL(const Win32::DWORD ttl)
			{
				if (!m_socket || m_socket == InvalidSocket)
					throw WinSockError("Not in a valid state to set TTL support");

				Win32::DWORD layer;
				Win32::DWORD argument;
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
						throw WinSockError("Unknown address family");
				}

				// Query support for the argument
				Win32::DWORD optVal;
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
					const auto lastError = Win32::WinSock::WSAGetLastError();
					Error::ThrowNested(
						Error::Win32Error("getsockopt() failed", lastError, L"ws2_32.dll"),
						WinSockError("TTL option is not supported")
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
					const auto lastError = Win32::WinSock::WSAGetLastError();
					Error::ThrowNested(
						Error::Win32Error("setsockopt() failed", lastError, L"ws2_32.dll"),
						WinSockError("Failed to set option")
					);
				}
			}

			void SetMaxRetryTimeout(const Win32::DWORD timeoutSeconds)
			{
				if (!m_socket || m_socket == InvalidSocket)
					throw WinSockError("Not in a valid state to set TTL support");

				// Query support for the argument
				Win32::DWORD optVal;
				int optLen = sizeof(optVal);
				int optResult = getsockopt(
					m_socket,
					Win32::WinSock::IPPROTO::IPPROTO_TCP,
					Win32::WinSock::_TCP_MAXRT,
					reinterpret_cast<char*>(&optVal),
					&optLen
				);
				if (optResult == Win32::WinSock::_SOCKET_ERROR)
				{
					const auto lastError = Win32::WinSock::WSAGetLastError();
					Error::ThrowNested(
						Error::Win32Error("getsockopt() failed", lastError, L"ws2_32.dll"),
						WinSockError("RT option is not supported")
					);
				}

				optVal = timeoutSeconds;
				optResult = Win32::WinSock::setsockopt(
					m_socket,
					Win32::WinSock::IPPROTO::IPPROTO_TCP,
					Win32::WinSock::_TCP_MAXRT,
					reinterpret_cast<char*>(&optVal),
					optLen
				);
				if (optResult == Win32::WinSock::_SOCKET_ERROR)
				{
					const auto lastError = Win32::WinSock::WSAGetLastError();
					Error::ThrowNested(
						Error::Win32Error("setsockopt() failed", lastError, L"ws2_32.dll"),
						WinSockError("Failed to set RT option")
					);
				}
			}

		public:
			const std::wstring& GetHost() const noexcept
			{
				return m_host;
			}

			unsigned GetPort() const noexcept
			{
				return m_portNumber;
			}

			SOCKET GetHandle() const noexcept
			{
				return m_socket;
			}

		private:
			TCPSocket& Move(TCPSocket& other) noexcept
			{
				Close();
				m_host = std::move(other.m_host);
				m_portNumber = other.m_portNumber;
				m_socket = other.m_socket;
				other.m_socket = InvalidSocket;
				m_addressFamily = other.m_addressFamily;
				return *this;
			}

		private:
			std::wstring m_host;
			unsigned m_portNumber = 0;
			Win32::WinSock::SOCKET m_socket = Win32::WinSock::_INVALID_SOCKET; // doesn't work with unique_ptr
			int m_addressFamily = 0;
			AddrInfoWUniquePtr m_addrPtr;
	};		
	
}