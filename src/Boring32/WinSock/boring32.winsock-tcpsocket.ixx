export module boring32.winsock:tcpsocket;
import :uniqueptrs;
import std;

import <win32.hpp>;
import :winsockerror;
import boring32.error;
import boring32.strings;


export namespace Boring32::WinSock
{

	// Based on https://docs.microsoft.com/en-us/windows/win32/winsock/winsock-client-application
	class TCPSocket
	{
		public:
			static const SOCKET InvalidSocket;

		public:
			virtual ~TCPSocket()
			{
				Close();
			}

			TCPSocket() = default;

			TCPSocket(const TCPSocket& other) = delete;

			TCPSocket(TCPSocket&& other) noexcept
			{
				Move(other);
			}

			TCPSocket(const std::wstring host, const unsigned portNumber)
				: m_host(std::move(host)),
				m_portNumber(portNumber)
			{ }

		public:
			virtual TCPSocket& operator=(const TCPSocket& other) = delete;
			virtual TCPSocket& operator=(TCPSocket&& other) noexcept
			{
				return Move(other);
			}

		public:
			virtual void Open()
			{
				if (m_socket && m_socket != InvalidSocket)
					return;

				ADDRINFOW hints{
					.ai_family = AF_INET,
					.ai_socktype = SOCK_STREAM,
					.ai_protocol = IPPROTO_TCP
				};

				ADDRINFOW* addrInfoResult;
				// https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getaddrinfow
				std::wstring portNumber = m_portNumber ? std::to_wstring(m_portNumber) : L"";
				const int status = GetAddrInfoW(
					m_host.c_str(),
					portNumber.c_str(),
					nullptr,
					&addrInfoResult
				);
				if (status) Error::ThrowNested(
					Error::Win32Error("GetAddrInfoW() failed", status, L"ws2_32.dll"),
					WinSockError("Failed to get address info")
				);
				if (!addrInfoResult)
					throw WinSockError("GetAddrInfoW() did not find any valid interfaces");
				m_addrPtr = AddrInfoWUniquePtr(addrInfoResult);

				// https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-socket
				m_socket = socket(
					addrInfoResult->ai_family,
					addrInfoResult->ai_socktype,
					addrInfoResult->ai_protocol
				);
				if (m_socket == InvalidSocket)
				{
					const auto lastError = WSAGetLastError();
					Error::ThrowNested(
						Error::Win32Error("socket() failed", lastError, L"ws2_32.dll"),
						WinSockError("Failed to open socket")
					);
				}

				m_addressFamily = addrInfoResult->ai_family;
			}

			virtual void Connect()
			{
				Connect(0, 0);
			}

			virtual void Connect(const DWORD socketTTL, const DWORD maxRetryTimeout)
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
				const int connectionResult = connect(
					m_socket,
					m_addrPtr->ai_addr,
					static_cast<int>(m_addrPtr->ai_addrlen)
				);
				if (connectionResult == SOCKET_ERROR)
				{
					const auto lastError = WSAGetLastError();
					Error::ThrowNested(
						Error::Win32Error("connect() failed", lastError, L"ws2_32.dll"),
						WinSockError("Failed to connect socket")
					);
				}
			}

			virtual void Close()
			{
				if (!m_socket)
					return;
				if (m_socket == InvalidSocket)
					return;

				closesocket(m_socket);
				m_socket = InvalidSocket;
			}

			virtual void Send(const std::vector<std::byte>& data)
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
					const int sentBytes = send(
						m_socket,
						bufferStart,
						static_cast<int>(remainingBytes),
						0
					);
					if (sentBytes == SOCKET_ERROR)
					{
						const auto lastError = WSAGetLastError();
						Error::ThrowNested(
							Error::Win32Error("send() failed", lastError, L"ws2_32.dll"),
							WinSockError("Failed to send data through socket")
						);
					}
					totalSentBytes += sentBytes;
				}
			}

			virtual std::vector<std::byte> Receive(const unsigned bytesToRead)
			{
				if (!m_socket || m_socket == InvalidSocket)
					throw WinSockError("Socket is not valid");
				if (!bytesToRead)
					return {};

				// https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-recv
				std::vector<std::byte> recvbuf(bytesToRead);
				const int actualBytesRead = recv(m_socket, reinterpret_cast<char*>(&recvbuf[0]), bytesToRead, 0);
				if (actualBytesRead < 0)
				{
					const auto lastError = WSAGetLastError();
					Error::ThrowNested(
						Error::Win32Error("recv() failed", lastError, L"ws2_32.dll"),
						WinSockError("Failed to receive data through socket")
					);
				}

				recvbuf.resize(actualBytesRead);
				return recvbuf;
			}

			virtual void SetSocketTTL(const DWORD ttl)
			{
				if (!m_socket || m_socket == InvalidSocket)
					throw WinSockError("Not in a valid state to set TTL support");

				DWORD layer;
				DWORD argument;
				switch (m_addressFamily)
				{
				case AF_INET:
					layer = IPPROTO_IP;
					argument = IP_TTL;
					break;

				case AF_INET6:
					layer = IPPROTO_IPV6;
					argument = IPV6_UNICAST_HOPS;
					break;

				default:
					throw WinSockError("Unknown address family");
				}

				// Query support for the argument
				DWORD optVal;
				int optLen = sizeof(optVal);
				int optResult = getsockopt(
					m_socket,
					layer,
					argument,
					reinterpret_cast<char*>(&optVal),
					&optLen
				);
				if (optResult == SOCKET_ERROR)
				{
					const auto lastError = WSAGetLastError();
					Error::ThrowNested(
						Error::Win32Error("getsockopt() failed", lastError, L"ws2_32.dll"),
						WinSockError("TTL option is not supported")
					);
				}

				// Actually set the argument
				optVal = ttl;
				optResult = setsockopt(
					m_socket,
					layer,
					argument,
					reinterpret_cast<char*>(&optVal),
					optLen
				);
				if (optResult == SOCKET_ERROR)
				{
					const auto lastError = WSAGetLastError();
					Error::ThrowNested(
						Error::Win32Error("setsockopt() failed", lastError, L"ws2_32.dll"),
						WinSockError("Failed to set option")
					);
				}
			}

			virtual void SetMaxRetryTimeout(const DWORD timeoutSeconds)
			{
				if (!m_socket || m_socket == InvalidSocket)
					throw WinSockError("Not in a valid state to set TTL support");

				// Query support for the argument
				DWORD optVal;
				int optLen = sizeof(optVal);
				int optResult = getsockopt(
					m_socket,
					IPPROTO_TCP,
					TCP_MAXRT,
					reinterpret_cast<char*>(&optVal),
					&optLen
				);
				if (optResult == SOCKET_ERROR)
				{
					const auto lastError = WSAGetLastError();
					Error::ThrowNested(
						Error::Win32Error("getsockopt() failed", lastError, L"ws2_32.dll"),
						WinSockError("RT option is not supported")
					);
				}

				optVal = timeoutSeconds;
				optResult = setsockopt(
					m_socket,
					IPPROTO_TCP,
					TCP_MAXRT,
					reinterpret_cast<char*>(&optVal),
					optLen
				);
				if (optResult == SOCKET_ERROR)
				{
					const auto lastError = WSAGetLastError();
					Error::ThrowNested(
						Error::Win32Error("setsockopt() failed", lastError, L"ws2_32.dll"),
						WinSockError("Failed to set RT option")
					);
				}
			}

		public:
			virtual const std::wstring& GetHost() const noexcept
			{
				return m_host;
			}

			virtual unsigned GetPort() const noexcept
			{
				return m_portNumber;
			}

			virtual SOCKET GetHandle() const noexcept
			{
				return m_socket;
			}

		protected:
			virtual TCPSocket& Move(TCPSocket& other) noexcept
			{
				Close();
				m_host = std::move(other.m_host);
				m_portNumber = other.m_portNumber;
				m_socket = other.m_socket;
				other.m_socket = InvalidSocket;
				m_addressFamily = other.m_addressFamily;

				return *this;
			}

		protected:
			std::wstring m_host;
			unsigned m_portNumber = 0;
			SOCKET m_socket = INVALID_SOCKET; // doesn't work with unique_ptr
			int m_addressFamily = 0;
			AddrInfoWUniquePtr m_addrPtr;
	};		
	
	const SOCKET TCPSocket::InvalidSocket = INVALID_SOCKET;
}