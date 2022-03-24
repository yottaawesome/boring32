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

module boring32.winsock.socket;
import boring32.winsock.winsockerror;
import boring32.error.errorbase;
import boring32.strings;

namespace Boring32::WinSock
{	
	Socket::~Socket()
	{
		Close();
	}

	Socket::Socket(const std::wstring host, const unsigned portNumber)
		: m_host(std::move(host)),
		m_portNumber(portNumber),
		m_preconnectTTL(0)
	{ }

	Socket::Socket(Socket&& other) noexcept = default;

	Socket& Socket::operator=(Socket&& other) noexcept = default;

	void Socket::SetSocketTTL(const DWORD ttl)
	{
		if (!m_socket || m_socket == INVALID_SOCKET) throw WinSockError(
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

	void Socket::SetPreconnectTTL(const DWORD ttl)
	{
		m_preconnectTTL = ttl;
	}

	void Socket::Connect()
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/ws2def/ns-ws2def-addrinfow
		ADDRINFOW hints
		{
			.ai_family = AF_INET | AF_INET6,
			.ai_socktype = SOCK_STREAM,
			.ai_protocol = IPPROTO_TCP
		};

		ADDRINFOW* addrResult;
		// https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getaddrinfow
		const int status = GetAddrInfoW(
			m_host.c_str(),
			m_portNumber ? std::to_wstring(m_portNumber).c_str() : nullptr,
			&hints,
			&addrResult
		);
		if (status)
			throw WinSockError(std::source_location::current(), "GetAddreInfoW() failed", status);

		AddrInfoWUniquePtr addrPtr = AddrInfoWUniquePtr(addrResult);
		// Attempt to connect to an address until one succeeds
		m_socket = INVALID_SOCKET;
		for (ADDRINFOW* currentAddr = addrResult; currentAddr != nullptr && m_socket == INVALID_SOCKET; currentAddr = currentAddr->ai_next)
		{
			// Create a SOCKET for connecting to server
			m_socket = socket(
				currentAddr->ai_family,
				currentAddr->ai_socktype,
				currentAddr->ai_protocol
			);
			if (m_socket == INVALID_SOCKET) throw WinSockError(
				std::source_location::current(), 
				"socket() failed", 
				WSAGetLastError()
			);
			m_addressFamily = currentAddr->ai_family;
			if (m_preconnectTTL)
				SetSocketTTL(m_preconnectTTL);
			// Connect to server.
			const int connectionResult = connect(
				m_socket,
				currentAddr->ai_addr, 
				static_cast<int>(currentAddr->ai_addrlen)
			);
			// Connected successfully.
			if (connectionResult != SOCKET_ERROR)
			{
				m_addressFamily = currentAddr->ai_family;
			}
			else
			{
				// Couldn't connect; free the socket and try the next entry.
				if (closesocket(m_socket) == SOCKET_ERROR) throw WinSockError(
					std::source_location::current(),
					"closesocket() failed",
					WSAGetLastError()
				);
				m_socket = INVALID_SOCKET;
			}
		}

		// Failed connecting in all cases.
		if (m_socket == INVALID_SOCKET)
		{
			const std::string errorMsg = std::format(
				"Failed connecting to server {}:{}", 
				Strings::ConvertString(m_host), 
				m_portNumber
			);
			throw WinSockError(std::source_location::current(), errorMsg);
		}
	}

	void Socket::Close()
	{
		if (m_socket && m_socket != INVALID_SOCKET)
			closesocket(m_socket);
	}

	void Socket::Send(const std::vector<std::byte>& data)
	{
		if (!m_socket || m_socket == INVALID_SOCKET)
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
		if (!m_socket || m_socket == INVALID_SOCKET)
			throw Error::ErrorBase<std::runtime_error>(std::source_location::current(), "Socket is not valid");

		// https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-recv
		std::vector<std::byte> recvbuf(bytesToRead);
		const int actualBytesRead = recv(m_socket, reinterpret_cast<char*>(&recvbuf[0]), bytesToRead, 0);
		if (actualBytesRead < 0)
			throw WinSockError(std::source_location::current(), "recv() failed", WSAGetLastError());

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
}