#include "pch.hpp"
#include <stdexcept>
#include <vector>
#include <sstream>
#include "include/WinHttp/WebSocket.hpp"
#include "include/Error/Error.hpp"

namespace Boring32::WinHttp
{
	WebSocket::~WebSocket()
	{
		Close();
	}

	// Adapted from https://stackoverflow.com/a/29752943/7448661
	std::wstring Replace(std::wstring source, const std::wstring& from, const std::wstring& to)
	{
		std::wstring newString;
		newString.reserve(source.length());

		std::wstring::size_type lastPos = 0;
		std::wstring::size_type findPos;

		while (std::wstring::npos != (findPos = source.find(from, lastPos)))
		{
			newString.append(source, lastPos, findPos - lastPos);
			newString += to;
			lastPos = findPos + from.length();
		}

		newString += source.substr(lastPos);

		return newString;
	}

	WebSocket::WebSocket(std::wstring server, const UINT port, const bool ignoreSslErrors)
	:	m_server(std::move(server)),
		m_port(port),
		m_ignoreSslErrors(ignoreSslErrors),
		m_hSession(nullptr),
		m_hConnect(nullptr),
		m_status(WebSocketStatus::NotInitialised)
	{
		CleanServerString();
	}

	const std::wstring& WebSocket::GetServer()
	{
		return m_server;
	}

	void WebSocket::SetServer(const std::wstring& newServer, const UINT port, const bool ignoreSslErrors)
	{
		m_server = newServer;
		m_port = port;
		m_ignoreSslErrors = ignoreSslErrors;
		CleanServerString();
	}

	void WebSocket::CleanServerString()
	{
		std::wstring whatToReplace1 = L"http://";
		std::wstring whatToReplace2 = L"https://";
		m_server =
			Replace(
				Replace(
					m_server,
					whatToReplace1,
					L""
				),
				whatToReplace2,
				L""
			);
	}

	void WebSocket::Connect(const std::wstring& path)
	{
		InternalConnect(path);
	}

	void WebSocket::Connect()
	{
		InternalConnect(L"");
	}

	void WebSocket::InternalConnect(const std::wstring& path)
	{
		if (m_status != WebSocketStatus::NotInitialised)
			throw std::runtime_error("WebSocket needs to be in NotInitialised state to connect");

		m_hSession = WinHttpOpen(
			L"Sinefa Windows Agent/1.0",
			WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
			WINHTTP_NO_PROXY_NAME,
			WINHTTP_NO_PROXY_BYPASS,
			0
		);
		if (m_hSession == nullptr)
			throw std::runtime_error("WinHttpOpen failed");

		m_hConnect = WinHttpConnect(
			m_hSession.Get(),
			m_server.c_str(),
			m_port,
			0);

		WinHttpHandle requestHandle = WinHttpOpenRequest(
			m_hConnect.Get(),
			L"GET",
			path.c_str(),
			nullptr,
			nullptr,
			nullptr,
			WINHTTP_FLAG_SECURE);

		if (m_ignoreSslErrors)
		{
			DWORD dwFlags =
				SECURITY_FLAG_IGNORE_UNKNOWN_CA |
				SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE |
				SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
				SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
			bool security = WinHttpSetOption(
				requestHandle.Get(),
				WINHTTP_OPTION_SECURITY_FLAGS,
				&dwFlags,
				sizeof(dwFlags)
			);
			if (!security)
			{
				m_status = WebSocketStatus::Error;
				throw Error::Win32Exception("Failed to set security options");
			}
		}

		bool websocket = WinHttpSetOption(
			requestHandle.Get(),
			WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET,
			NULL,
			0);
		if (!websocket)
		{
			m_status = WebSocketStatus::Error;
			throw Error::Win32Exception("Failed to set web socket upgrade option");
		}

		bool isSuccessful = WinHttpSendRequest(
			requestHandle.Get(),
			WINHTTP_NO_ADDITIONAL_HEADERS,
			0,
			nullptr,
			0,
			0,
			0);
		if (!isSuccessful)
		{
			m_status = WebSocketStatus::Error;
			throw Error::Win32Exception("Failed to send web socket request");
		}

		isSuccessful = WinHttpReceiveResponse(requestHandle.Get(), 0);
		if (!isSuccessful)
		{
			m_status = WebSocketStatus::Error;
			throw Error::Win32Exception("Failed to receive web socket response");
		}

		DWORD statusCode = 0;
		DWORD statusCodeSize = sizeof(statusCode);
		WinHttpQueryHeaders(
			requestHandle.Get(),
			WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
			WINHTTP_HEADER_NAME_BY_INDEX,
			&statusCode,
			&statusCodeSize,
			WINHTTP_NO_HEADER_INDEX);
		if (statusCode != 101) // switching protocol
		{
			std::stringstream ss;
			ss
				<< "Received unexpected HTTP respons code while upgrading to websocket: "
				<< std::to_string(statusCode);
			throw std::runtime_error(ss.str());
		}

		m_webSocketHandle = WinHttpWebSocketCompleteUpgrade(requestHandle.Get(), 0);
		if (m_webSocketHandle == nullptr)
		{
			m_status = WebSocketStatus::Error;
			throw Error::Win32Exception("Failed to complete web socket upgrade");
		}

		requestHandle = nullptr;
		m_status = WebSocketStatus::Connected;
	}

	WebSocketStatus WebSocket::GetStatus()
	{
		return m_status;
	}

	void WebSocket::SendString(const std::string& msg)
	{
		if (m_status != WebSocketStatus::Connected)
			throw std::runtime_error("WebSocket is not connected to send data");

		DWORD dwError = WinHttpWebSocketSend(
			m_webSocketHandle.Get(),
			WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
			(PVOID)&msg[0],
			msg.size() * sizeof(char)
		);
		if (dwError != ERROR_SUCCESS)
		{
			m_status = WebSocketStatus::Error;
			throw std::runtime_error(std::to_string(GetLastError()));
		}
	}

	void WebSocket::SendBuffer(const std::vector<char>& buffer)
	{
		if (m_status != WebSocketStatus::Connected)
			throw std::runtime_error("WebSocket is not connected to send data");

		DWORD dwError = WinHttpWebSocketSend(
			m_webSocketHandle.Get(),
			WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE,
			(PVOID)&buffer[0],
			buffer.size() * sizeof(char)
		);
		if (dwError != ERROR_SUCCESS)
		{
			m_status = WebSocketStatus::Error;
			throw std::runtime_error(std::to_string(GetLastError()));
		}
	}

	bool WebSocket::Receive(std::vector<char>& receiveBuffer)
	{
		if (m_status != WebSocketStatus::Connected)
			throw std::runtime_error("WebSocket is not connected to receive data");

		constexpr UINT bufferBlockSize = 2048;
		receiveBuffer.clear();
		receiveBuffer.resize(bufferBlockSize);
		WINHTTP_WEB_SOCKET_BUFFER_TYPE bufferType;
		DWORD bufferLength = receiveBuffer.size() * sizeof(char);
		DWORD totalBytesTransferred = 0;
		char* currentBufferPointer = &receiveBuffer[0];
		
		while (true);
		{
			DWORD dwBytesTransferred = 0;
			DWORD statusCode = WinHttpWebSocketReceive(
				m_webSocketHandle.Get(),
				currentBufferPointer,
				bufferLength,
				&dwBytesTransferred,
				&bufferType);
			// If the server terminates the connection, 12030 will returned.
			if (statusCode != ERROR_SUCCESS)
				throw Error::Win32Exception("Connection error when receiving websocket data");
			
			// The server closed the connection.
			if (bufferType == WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE)
			{
				Close();
				return false;
			}

			currentBufferPointer += dwBytesTransferred;
			bufferLength -= dwBytesTransferred;
			totalBytesTransferred += dwBytesTransferred;

			// We've now got a complete buffer of either binary or UTF8 type.
			if (
				bufferType == WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE
				|| bufferType == WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE
			)
			{
				receiveBuffer.resize(totalBytesTransferred);
				return true;
			}

			if (bufferLength == 0)
			{
				receiveBuffer.resize(receiveBuffer.size() + bufferBlockSize);
				bufferLength = receiveBuffer.size() * sizeof(char);
			}
		}
	}

	void WebSocket::Close()
	{
		WinHttpWebSocketClose(m_webSocketHandle.Get(), WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, nullptr, 0);
		m_status = WebSocketStatus::Closed;
	}
}