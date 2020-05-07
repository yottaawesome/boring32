#include "pch.hpp"
#include <stdexcept>
#include <vector>
#include <functional>
#include <sstream>
#include "include/Win32Utils.hpp"

namespace Win32Utils::WinHttp
{
	WinHttpWebSocket::~WinHttpWebSocket()
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

	WinHttpWebSocket::WinHttpWebSocket(std::wstring server, const UINT port, const bool ignoreSslErrors)
	:	m_server(std::move(server)),
		m_port(port),
		m_ignoreSslErrors(ignoreSslErrors),
		m_hSession(nullptr),
		m_hConnect(nullptr),
		m_status(WinHttpWebSocketStatus::NotInitialised)
	{
		CleanServerString();
	}

	const std::wstring& WinHttpWebSocket::GetServer()
	{
		return m_server;
	}

	void WinHttpWebSocket::SetServer(const std::wstring& newServer, const UINT port, const bool ignoreSslErrors)
	{
		m_server = newServer;
		m_port = port;
		m_ignoreSslErrors = ignoreSslErrors;
		CleanServerString();
	}

	void WinHttpWebSocket::CleanServerString()
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

	void WinHttpWebSocket::Connect(const std::wstring& path)
	{
		InternalConnect(path);
	}

	void WinHttpWebSocket::Connect()
	{
		InternalConnect(L"");
	}

	void WinHttpWebSocket::InternalConnect(const std::wstring& path)
	{
		if (m_status != WinHttpWebSocketStatus::NotInitialised)
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

		WinHttpHandle m_hRequestHandle = WinHttpOpenRequest(
			m_hConnect.Get(),
			L"GET",
			path.c_str(),
			NULL,
			NULL,
			NULL,
			WINHTTP_FLAG_SECURE);

		if (m_ignoreSslErrors)
		{
			DWORD dwFlags =
				SECURITY_FLAG_IGNORE_UNKNOWN_CA |
				SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE |
				SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
				SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
			bool security = WinHttpSetOption(
				m_hRequestHandle.Get(),
				WINHTTP_OPTION_SECURITY_FLAGS,
				&dwFlags,
				sizeof(dwFlags)
			);
			if (!security)
			{
				m_status = WinHttpWebSocketStatus::Error;
				throw std::runtime_error(std::to_string(GetLastError()));
			}
		}

		bool websocket = WinHttpSetOption(
			m_hRequestHandle.Get(),
			WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET,
			NULL,
			0);
		if (!websocket)
		{
			m_status = WinHttpWebSocketStatus::Error;
			throw std::runtime_error(std::to_string(GetLastError()));
		}

		BOOL fStatus = WinHttpSendRequest(
			m_hRequestHandle.Get(),
			WINHTTP_NO_ADDITIONAL_HEADERS,
			0,
			nullptr,
			0,
			0,
			0);
		if (!fStatus)
		{
			m_status = WinHttpWebSocketStatus::Error;
			throw std::runtime_error(std::to_string(GetLastError()));
		}

		fStatus = WinHttpReceiveResponse(m_hRequestHandle.Get(), 0);
		if (!fStatus)
		{
			m_status = WinHttpWebSocketStatus::Error;
			throw std::runtime_error(std::to_string(GetLastError()));
		}

		DWORD statusCode = 0;
		DWORD statusCodeSize = sizeof(statusCode);
		WinHttpQueryHeaders(
			m_hRequestHandle.Get(),
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

		m_hWebSocketHandle = WinHttpWebSocketCompleteUpgrade(m_hRequestHandle.Get(), NULL);
		if (m_hWebSocketHandle == nullptr)
		{
			m_status = WinHttpWebSocketStatus::Error;
			throw std::runtime_error(std::to_string(GetLastError()));
		}

		m_hRequestHandle = nullptr;
		m_status = WinHttpWebSocketStatus::Connected;
	}

	WinHttpWebSocketStatus WinHttpWebSocket::GetStatus()
	{
		return m_status;
	}

	void WinHttpWebSocket::SendString(const std::string& msg)
	{
		if (m_status != WinHttpWebSocketStatus::Connected)
			throw std::runtime_error("WebSocket is not connected to send data");

		DWORD dwError = WinHttpWebSocketSend(
			m_hWebSocketHandle.Get(),
			WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
			(PVOID)&msg[0],
			msg.size() * sizeof(char)
		);
		if (dwError != ERROR_SUCCESS)
		{
			m_status = WinHttpWebSocketStatus::Error;
			throw std::runtime_error(std::to_string(GetLastError()));
		}
	}

	void WinHttpWebSocket::SendBuffer(const std::vector<char>& buffer)
	{
		if (m_status != WinHttpWebSocketStatus::Connected)
			throw std::runtime_error("WebSocket is not connected to send data");

		DWORD dwError = WinHttpWebSocketSend(
			m_hWebSocketHandle.Get(),
			WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE,
			(PVOID)&buffer[0],
			buffer.size() * sizeof(char)
		);
		if (dwError != ERROR_SUCCESS)
		{
			m_status = WinHttpWebSocketStatus::Error;
			throw std::runtime_error(std::to_string(GetLastError()));
		}
	}

	bool WinHttpWebSocket::Receive(std::string& charBuffer)
	{
		if (m_status != WinHttpWebSocketStatus::Connected)
			throw std::runtime_error("WebSocket is not connected to receive data");

		charBuffer.clear();
		charBuffer.resize(2048);
		WINHTTP_WEB_SOCKET_BUFFER_TYPE eBufferType;
		DWORD dwBufferLength = charBuffer.size() * sizeof(char);
		char* pbCurrentBufferPointer = &charBuffer[0];
		do
		{
			DWORD dwBytesTransferred = 0;
			DWORD dwError = WinHttpWebSocketReceive(
				m_hWebSocketHandle.Get(),
				pbCurrentBufferPointer,
				dwBufferLength,
				&dwBytesTransferred,
				&eBufferType);
			// If the server terminates the connection, 12030 will returned.
			if (dwError != ERROR_SUCCESS)
				throw std::runtime_error(std::to_string(GetLastError()));
			if (eBufferType == WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE)
			{
				Close();
				return false;
			}
			{
				pbCurrentBufferPointer += dwBytesTransferred;
				dwBufferLength -= dwBytesTransferred;
			}			
		} while (eBufferType == WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE || eBufferType == WINHTTP_WEB_SOCKET_BINARY_FRAGMENT_BUFFER_TYPE);

		charBuffer.shrink_to_fit();
		return true;
	}

	void WinHttpWebSocket::Close()
	{
		WinHttpWebSocketClose(m_hWebSocketHandle.Get(), WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, nullptr, 0);
		m_status = WinHttpWebSocketStatus::Closed;
	}
}