module;

#include <string>
#include <vector>
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
		SOCKET connectSocket = INVALID_SOCKET;
		for (ADDRINFOW* currentAddr = addrResult; currentAddr != nullptr; currentAddr = currentAddr->ai_next)
		{
			// Create a SOCKET for connecting to server
			connectSocket = socket(
				currentAddr->ai_family,
				currentAddr->ai_socktype,
				currentAddr->ai_protocol
			);
			if (connectSocket == INVALID_SOCKET)
				throw WinSockError(
					std::source_location::current(), 
					"socket() failed", 
					WSAGetLastError()
				);

			// Connect to server.
			const int connectionResult = connect(
				connectSocket, 
				currentAddr->ai_addr, 
				static_cast<int>(currentAddr->ai_addrlen)
			);
			// Connected successfully.
			if (connectionResult != SOCKET_ERROR)
				break;
			// Couldn't connect; free the socket and try the next entry.
			if (closesocket(connectSocket) == SOCKET_ERROR)
				throw WinSockError(
					std::source_location::current(), 
					"closesocket() failed", 
					WSAGetLastError()
				);
			connectSocket = INVALID_SOCKET;
		}
		// Failed connecting in all cases.
		if (connectSocket == INVALID_SOCKET)
		{
			const std::string errorMsg = std::format(
				"Failed connecting to server {}:{}", 
				Strings::ConvertString(m_host), 
				m_portNumber
			);
			throw Error::ErrorBase<std::runtime_error>(std::source_location::current(), errorMsg);
		}
			
		m_socket = connectSocket;
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