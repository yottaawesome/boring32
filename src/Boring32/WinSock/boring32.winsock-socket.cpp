module;

#include <string>
#include <vector>
#include <iostream>
#include <format>
#include <source_location>
#include <stdexcept>
#include <Windows.h>
#include <winsock2.h>
#include <Ws2tcpip.h>

module boring32.winsock:socket;
import :winsockerror;
import boring32.error;
import boring32.strings;

namespace Boring32::WinSock
{	
	const SOCKET Socket::InvalidSocket = INVALID_SOCKET;

	Socket::~Socket()
	{
		Close();
	}

	Socket::Socket()
		: m_portNumber(0),
		m_socket(InvalidSocket),
		m_addressFamily(0)
	{ }

	Socket::Socket(const std::wstring host, const unsigned portNumber)
		: m_host(std::move(host)),
		m_portNumber(portNumber),
		m_socket(InvalidSocket),
		m_addressFamily(0)
	{ }

	Socket::Socket(Socket&& other) noexcept
	{
		Move(other);
	}

	Socket& Socket::operator=(Socket&& other) noexcept
	{
		return Move(other);
	}

	Socket& Socket::Move(Socket& other) noexcept
	{
		Close();
		m_host = std::move(other.m_host);
		m_portNumber = other.m_portNumber;
		m_socket = other.m_socket;
		other.m_socket = InvalidSocket;
		m_addressFamily = other.m_addressFamily;

		return *this;
	}

	void Socket::SetSocketTTL(const DWORD ttl)
	{
		if (!m_socket || m_socket == InvalidSocket) throw WinSockError(
			std::source_location::current(),
			"Not in a valid state to set TTL support"
		);

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
				throw WinSockError(std::source_location::current(), "Unknown address family");
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
		if (optResult == SOCKET_ERROR) throw WinSockError(
			std::source_location::current(),
			"TTL option is not support",
			WSAGetLastError()
		);

		// Actually set the argument
		optVal = ttl;
		optResult = setsockopt(
			m_socket,
			layer,
			argument,
			reinterpret_cast<char*>(&optVal),
			optLen
		);
		if (optResult == SOCKET_ERROR) throw WinSockError(
			std::source_location::current(),
			"setsockopt() failed",
			WSAGetLastError()
		);
	}
	
	void Socket::Connect()
	{
		Connect(0, 0);
	}

	void Socket::Open()
	{
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
		if (status) throw WinSockError(
			std::source_location::current(), 
			"GetAddrInfoW() failed", 
			status
		);
		if (!addrInfoResult) throw WinSockError(
			std::source_location::current(), 
			"GetAddrInfoW() did not find any valid interfaces", 
			status
		);
		m_addrPtr = AddrInfoWUniquePtr(addrInfoResult);

		// https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-socket
		m_socket = socket(
			addrInfoResult->ai_family,
			addrInfoResult->ai_socktype,
			addrInfoResult->ai_protocol
		);
		if (m_socket == InvalidSocket) throw WinSockError(
			std::source_location::current(),
			"socket() failed",
			WSAGetLastError()
		);

		m_addressFamily = addrInfoResult->ai_family;
	}

	void Socket::Connect(const DWORD socketTTL, const DWORD maxRetryTimeout)
	{
		if (socketTTL)
			SetSocketTTL(socketTTL);
		if (maxRetryTimeout)
			SetMaxRetryTimeout(maxRetryTimeout);

		// Trying to connect to a random socket on the remote host seems to trigger a long timeout
		// ending with error code 0X0000274C=10060=WSAETIMEDOUT. Can we adjust this timeout
		// and is it possible get a different error like WSAECONNREFUSED?
		// https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-connect
		const int connectionResult = connect(
			m_socket,
			m_addrPtr->ai_addr,
			static_cast<int>(m_addrPtr->ai_addrlen)
		);
		if (connectionResult == SOCKET_ERROR) throw WinSockError(
			std::source_location::current(),
			"connect() failed",
			WSAGetLastError()
		);
	}

	void Socket::Close()
	{
		if (!m_socket)
			return;
		if (m_socket == InvalidSocket)
			return;

		closesocket(m_socket);
		m_socket = InvalidSocket;
	}

	void Socket::Send(const std::vector<std::byte>& data)
	{
		if (!m_socket || m_socket == InvalidSocket)
			throw Error::ErrorBase<std::runtime_error>(std::source_location::current(), "Socket is not valid");

		// https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-send
		const int sentBytes = send(
			m_socket, 
			reinterpret_cast<char*>(const_cast<std::byte*>(&data[0])),
			static_cast<int>(data.size()), 
			0
		);
		if (sentBytes == SOCKET_ERROR)
			throw WinSockError(std::source_location::current(), "send() failed", WSAGetLastError());
	}

	std::vector<std::byte> Socket::Receive(const unsigned bytesToRead)
	{
		if (!m_socket || m_socket == InvalidSocket)
			throw WinSockError(std::source_location::current(), "Socket is not valid");

		// https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-recv
		std::vector<std::byte> recvbuf(bytesToRead);
		const int actualBytesRead = recv(m_socket, reinterpret_cast<char*>(&recvbuf[0]), bytesToRead, 0);
		if (actualBytesRead < 0) throw WinSockError(
			std::source_location::current(), 
			"recv() failed", 
			WSAGetLastError()
		);

		recvbuf.resize(actualBytesRead);
		return recvbuf;
	}

	const std::wstring& Socket::GetHost() const noexcept
	{
		return m_host;
	}

	unsigned Socket::GetPort() const noexcept
	{
		return m_portNumber;
	}

	SOCKET Socket::GetHandle() const noexcept
	{
		return m_socket;
	}

	void Socket::SetMaxRetryTimeout(const DWORD timeoutSeconds)
	{
		if (!m_socket || m_socket == InvalidSocket) throw WinSockError(
			std::source_location::current(),
			"Not in a valid state to set TTL support"
		);

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
		if (optResult == SOCKET_ERROR) throw WinSockError(
			std::source_location::current(),
			"RT option is not supported",
			WSAGetLastError()
		);

		optVal = timeoutSeconds;
		optResult = setsockopt(
			m_socket,
			IPPROTO_TCP,
			TCP_MAXRT,
			reinterpret_cast<char*>(&optVal),
			optLen
		);
		if (optResult == SOCKET_ERROR) throw WinSockError(
			std::source_location::current(),
			"setsockopt() failed",
			WSAGetLastError()
		);
	}
}