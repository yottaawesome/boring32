module;

#include <string>
#include <source_location>
#include <stdexcept>
#include <Windows.h>
#include <winsock2.h>
#include <Ws2tcpip.h>

module boring32.winsock.socket;
import boring32.winsock.winsockerror;

namespace Boring32::WinSock
{	
	Socket::~Socket()
	{
		Close();
	}

	Socket::Socket(const std::wstring host, const unsigned portNumber)
		: m_host(std::move(host)),
		m_portNumber(portNumber)
	{
	}

	Socket::Socket(Socket&& other) noexcept = default;

	Socket& Socket::operator=(Socket&& other) noexcept = default;

	void Socket::Connect()
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/ws2def/ns-ws2def-addrinfow
		ADDRINFOW hints
		{
			.ai_family = AF_UNSPEC,
			.ai_socktype = SOCK_STREAM,
			.ai_protocol = IPPROTO_TCP
		};

		ADDRINFOW* addrResult;
		// https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getaddrinfow
		int status = GetAddrInfoW(
			m_host.c_str(),
			m_portNumber ? std::to_wstring(m_portNumber).c_str() : nullptr,
			&hints,
			&addrResult
		);
		if (status)
			throw WinSockError(std::source_location::current(), "GetAddreInfoW() failed", status);
		AddrInfoWUniquePtr addrPtr = AddrInfoWUniquePtr(addrResult);

		// Attempt to connect to an address until one succeeds
		SOCKET connectSocket = INVALID_SOCKET;
		for (ADDRINFOW* ptr = addrResult; ptr != nullptr; ptr = ptr->ai_next)
		{
			// Create a SOCKET for connecting to server
			connectSocket = socket(
				ptr->ai_family,
				ptr->ai_socktype,
				ptr->ai_protocol
			);
			if (connectSocket == INVALID_SOCKET)
			{
				throw WinSockError(std::source_location::current(), "socket() failed", WSAGetLastError());
			}

			// Connect to server.
			int iResult = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult == SOCKET_ERROR)
			{
				closesocket(connectSocket);
				connectSocket = INVALID_SOCKET;
			}
			else
			{
				break;
			}
		}
		if (connectSocket == INVALID_SOCKET)
		{
			throw std::runtime_error("Failed connecting to server");
		}

		m_socket = connectSocket;
	}

	void Socket::Close()
	{
		if (m_socket && m_socket != INVALID_SOCKET)
			closesocket(m_socket);
	}
}