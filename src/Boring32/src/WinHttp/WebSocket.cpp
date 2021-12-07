#include "pch.hpp"
#include <stdexcept>
#include <vector>
#include <future>
#include <sstream>
#include <wincrypt.h>
#include "include/WinHttp/WebSockets/WebSocket.hpp"
#include "include/WinHttp/ProxyInfo.hpp"

import boring32.error.win32error;

namespace Boring32::WinHttp::WebSockets
{
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

	WebSocket::~WebSocket()
	{
		Close();
	}

	WebSocket::WebSocket()
	:	m_settings({}),
		m_winHttpConnection(nullptr),
		m_status(WebSocketStatus::NotInitialised)
	{ }

	WebSocket::WebSocket(WebSocketSettings settings)
	:	m_settings(std::move(settings)),
		m_winHttpConnection(nullptr),
		m_status(WebSocketStatus::NotInitialised)
	{ }

	WebSocket::WebSocket(WebSocket&& other) noexcept
	:	m_settings({}),
		m_winHttpConnection(nullptr),
		m_status(WebSocketStatus::NotInitialised)
	{
		Move(other);
	}

	WebSocket& WebSocket::operator=(WebSocket&& other) noexcept
	{
		Move(other);
		return *this;
	}
	
	void WebSocket::Move(WebSocket& other) noexcept
	{
		m_settings = std::move(other.m_settings);
		m_status = other.m_status;
		m_winHttpConnection = std::move(other.m_winHttpConnection);
		m_winHttpWebSocket = std::move(other.m_winHttpWebSocket);
	}

	const WebSocketSettings& WebSocket::GetSettings()
	{
		return m_settings;
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
			throw std::runtime_error(__FUNCSIG__ ": WebSocket needs to be in NotInitialised state to connect");

		try
		{
			if (m_settings.WinHttpSession.GetSession() == nullptr)
				throw std::runtime_error(__FUNCSIG__ ": WinHttp session cannot be null");

			m_winHttpConnection = WinHttpConnect(
				m_settings.WinHttpSession.GetSession(),
				m_settings.Server.c_str(),
				m_settings.Port,
				0
			);
			if (m_winHttpConnection == nullptr)
				throw Error::Win32Error(__FUNCSIG__ ": WinHttpConnect() failed", GetLastError());

			// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpopenrequest
			WinHttpHandle requestHandle = WinHttpOpenRequest(
				m_winHttpConnection.Get(),
				L"GET",
				path.c_str(),
				nullptr,
				nullptr,
				nullptr,
				WINHTTP_FLAG_SECURE
			);
			if (requestHandle == nullptr)
				throw Error::Win32Error(__FUNCSIG__ ": WinHttpOpenRequest() failed", GetLastError());

			bool success = false;
			if (m_settings.IgnoreSslErrors)
			{
				DWORD optionFlags = SECURITY_FLAG_IGNORE_ALL_CERT_ERRORS;
				// https://docs.microsoft.com/en-us/windows/win32/winhttp/option-flags
				// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpsetoption
				success = WinHttpSetOption(
					requestHandle.Get(),
					WINHTTP_OPTION_SECURITY_FLAGS,
					&optionFlags,
					sizeof(optionFlags)
				);
				if (success == false)
					throw Error::Win32Error(__FUNCSIG__ ": WinHttpSetOption() failed", GetLastError());
			}

			success = WinHttpSetOption(
				requestHandle.Get(),
				WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET,
				nullptr,
				0
			);
			if (success == false)
				throw Error::Win32Error(__FUNCSIG__ ": WinHttpSetOption() failed", GetLastError());

			if (m_settings.ClientCert.GetCert())
			{
				// If so, we need to set the certificate option, and retry the request.
				const bool setCertOption = WinHttpSetOption(
					requestHandle.Get(),
					WINHTTP_OPTION_CLIENT_CERT_CONTEXT,
					(void*)m_settings.ClientCert.GetCert(),
					sizeof(CERT_CONTEXT)
				);
				if (setCertOption == false)
					throw Error::Win32Error(__FUNCSIG__ ": WinHttpSetOption() failed for client certificate", GetLastError());
			}
			const wchar_t* connectionHeaders = m_settings.ConnectionHeaders.empty()
				? WINHTTP_NO_ADDITIONAL_HEADERS
				: m_settings.ConnectionHeaders.c_str();
			success = WinHttpSendRequest(
				requestHandle.Get(),
				connectionHeaders,
				-1L,
				nullptr,
				0,
				0,
				0
			);
			if (success == false)
				throw Error::Win32Error(
					__FUNCSIG__ ": WinHttpSendRequest() failed on initial request", 
					GetLastError()
				);

			success = WinHttpReceiveResponse(requestHandle.Get(), 0);
			if(success == false)
				throw Error::Win32Error(__FUNCSIG__ ": WinHttpReceiveResponse() failed on initial connection", GetLastError());

			DWORD statusCode = 0;
			DWORD statusCodeSize = sizeof(statusCode);
			// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpqueryheaders
			success = WinHttpQueryHeaders(
				requestHandle.Get(),
				WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
				WINHTTP_HEADER_NAME_BY_INDEX,
				&statusCode,
				&statusCodeSize,
				WINHTTP_NO_HEADER_INDEX
			);
			if (success == false)
				throw Error::Win32Error(__FUNCSIG__ ": WinHttpQueryHeaders() failed", GetLastError());

			if (statusCode != 101) // switching protocol
			{
				throw std::runtime_error(
					__FUNCSIG__
					": Received unexpected HTTP response code while upgrading to websocket: "
					+ std::to_string(statusCode)
				);
			}

			// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpwebsocketcompleteupgrade
			m_winHttpWebSocket = WinHttpWebSocketCompleteUpgrade(requestHandle.Get(), 0);
			if (m_winHttpWebSocket == nullptr)
				throw Error::Win32Error(__FUNCSIG__ ": WinHttpWebSocketCompleteUpgrade() failed", GetLastError());

			requestHandle = nullptr;
			m_status = WebSocketStatus::Connected;
		}
		catch (...)
		{
			m_status = WebSocketStatus::Error;
			throw;
		}
	}

	WebSocketStatus WebSocket::GetStatus() const noexcept
	{
		return m_status;
	}

	void WebSocket::SendString(const std::string& msg)
	{
		if (m_status != WebSocketStatus::Connected)
			throw std::runtime_error("WebSocket is not connected to send data");
		const void* a = reinterpret_cast<const void*>(&msg[0]);
		const DWORD statusCode = WinHttpWebSocketSend(
			m_winHttpWebSocket.Get(),
			WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
			reinterpret_cast<PVOID>(const_cast<char*>(&msg[0])),
			static_cast<DWORD>(msg.size() * sizeof(char))
		);
		if (statusCode != ERROR_SUCCESS)
		{
			m_status = WebSocketStatus::Error;
			throw Error::Win32Error("WebSocket::SendString(): WinHttpWebSocketSend() failed", statusCode);
		}
	}

	void WebSocket::SendBuffer(const std::vector<std::byte>& buffer)
	{
		if (m_status != WebSocketStatus::Connected)
			throw std::runtime_error("WebSocket is not connected to send data");

		const DWORD statusCode = WinHttpWebSocketSend(
			m_winHttpWebSocket.Get(),
			WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE,
			reinterpret_cast<PVOID>(const_cast<std::byte*>(&buffer[0])),
			static_cast<DWORD>(buffer.size() * sizeof(std::byte))
		);
		if (statusCode != ERROR_SUCCESS)
		{
			m_status = WebSocketStatus::Error;
			throw Error::Win32Error("WebSocket::SendString(): WinHttpWebSocketSend() failed", statusCode);
		}
	}
	
	std::shared_ptr<WebSocket::ReadResult> WebSocket::AsyncReceive()
	{
		auto result = std::make_shared<WebSocket::ReadResult>();
		// need to assign this as this causes the process to block if it goes out of scope
		result->Future = std::async(
			std::launch::async, 
			[this, result] 
			{
				try
				{
					this->Receive(result->Buffer);
					result->Succeeded = true;
				}
				catch (const std::exception& ex)
				{
					std::wcerr << ex.what() << std::endl;
				}
				result->Done.Signal(std::nothrow);
			});

		return result;
	}

	bool WebSocket::Receive(std::vector<char>& receiveBuffer)
	{
		if (m_status != WebSocketStatus::Connected)
			throw std::runtime_error("WebSocket is not connected to receive data");

		receiveBuffer.clear();
		receiveBuffer.resize(m_settings.BufferBlockSize);
		WINHTTP_WEB_SOCKET_BUFFER_TYPE bufferType;
		DWORD bufferLength = static_cast<DWORD>(receiveBuffer.size() * sizeof(char));
		DWORD totalBytesTransferred = 0;
		char* currentBufferPointer = &receiveBuffer[0];
		
		while (true)
		{
			DWORD bytesTransferred = 0;
			const DWORD statusCode = WinHttpWebSocketReceive(
				m_winHttpWebSocket.Get(),
				currentBufferPointer,
				bufferLength,
				&bytesTransferred,
				&bufferType);
			// If the server terminates the connection, 12030 will returned.
			if (statusCode != ERROR_SUCCESS)
				throw Error::Win32Error("Connection error when receiving websocket data", statusCode);
			
			// The server closed the connection.
			if (bufferType == WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE)
			{
				Close();
				return false;
			}

			currentBufferPointer += bytesTransferred;
			bufferLength -= bytesTransferred;
			totalBytesTransferred += bytesTransferred;

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
				receiveBuffer.resize(receiveBuffer.size() + m_settings.BufferBlockSize);
				bufferLength = static_cast<DWORD>(receiveBuffer.size() * sizeof(char));
				currentBufferPointer = &receiveBuffer[0] + totalBytesTransferred;
			}
		}
	}

	void WebSocket::Close()
	{
		m_status = WebSocketStatus::Closed;
		if (m_winHttpWebSocket != nullptr)
		{
			WinHttpWebSocketClose(
				m_winHttpWebSocket.Get(),
				WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS,
				nullptr,
				0
			);
		}
		m_winHttpWebSocket.Close();
		m_winHttpConnection.Close();
	}
}