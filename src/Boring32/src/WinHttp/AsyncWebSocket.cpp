#include "pch.hpp"
#include <algorithm>
#include "include/Strings/Strings.hpp"
#include "include/Error/Error.hpp"
#include "include/WinHttp/WebSockets/AsyncWebSocket.hpp"

namespace Boring32::WinHttp::WebSockets
{
	DWORD AsyncWebSocket::m_bufferBlockSize = 1024;

	AsyncWebSocket::~AsyncWebSocket()
	{
		if (m_status == WebSocketStatus::Connected) try
		{
			CloseSocket();
			int i = 0;
			while (i < 5 && m_status == WebSocketStatus::Closing)
			{
				Sleep(100);
				i++;
			}
		}
		catch (const std::exception& ex)
		{
			std::wcerr << __FUNCSIG__ ": " << ex.what() << std::endl;
		}
		DeleteCriticalSection(&m_cs);
	}

	AsyncWebSocket::AsyncWebSocket(const AsyncWebSocketSettings& settings)
	:	m_settings(settings),
		m_status(WebSocketStatus::NotInitialised),
		m_currentResult{}, 
		m_connectionResult{}
	{
		InitializeCriticalSection(&m_cs);
	}

	const AsyncWebSocketSettings& AsyncWebSocket::GetSettings()
	{
		return m_settings;
	}

	const ConnectionResult& AsyncWebSocket::Connect(const std::wstring& path)
	{
		return InternalConnect(path);
	}

	const ConnectionResult& AsyncWebSocket::Connect()
	{
		return InternalConnect(L"");
	}

	const ConnectionResult& AsyncWebSocket::GetConnectionStatus() const
	{
		return m_connectionResult;
	}

	void AsyncWebSocket::SendString(const std::string& msg)
	{
		if (m_status != WebSocketStatus::Connected)
			throw std::runtime_error("WebSocket is not connected to send data");

		const DWORD statusCode = WinHttpWebSocketSend(
			m_winHttpWebSocket.Get(),
			WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
			reinterpret_cast<void*>(const_cast<char*>(&msg[0])),
			static_cast<DWORD>(msg.size() * sizeof(char))
		);
		if (statusCode != ERROR_SUCCESS)
		{
			m_status = WebSocketStatus::Error;
			throw Error::Win32Error("WebSocket::SendString(): WinHttpWebSocketSend() failed", statusCode);
		}
	}

	void AsyncWebSocket::SendBuffer(const std::vector<std::byte>& buffer)
	{
		if (m_status != WebSocketStatus::Connected)
			throw std::runtime_error("WebSocket is not connected to send data");

		const DWORD statusCode = WinHttpWebSocketSend(
			m_winHttpWebSocket.Get(),
			WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE,
			reinterpret_cast<PVOID>(const_cast<std::byte*>(&buffer[0])),
			static_cast<DWORD>(buffer.size() * sizeof(char))
		);
		if (statusCode != ERROR_SUCCESS)
		{
			m_status = WebSocketStatus::Error;
			throw Error::Win32Error("WebSocket::SendString(): WinHttpWebSocketSend() failed", statusCode);
		}
	}

	const WebSocketReadResult& AsyncWebSocket::GetCurrentRead()
	{
		Async::CriticalSectionLock cs(m_cs);

		return m_currentResult;
	}

	WebSocketReadResult AsyncWebSocket::AsyncReceive(AsyncWebSocket* socket)
	{
		if (socket->m_status != WebSocketStatus::Connected)
			throw std::runtime_error("WebSocket is not connected to receive data");
		socket->m_currentResult.Complete.WaitOnEvent(INFINITE, true);
		return socket->m_currentResult;
	}

	std::shared_future<WebSocketReadResult> AsyncWebSocket::Receive2()
	{
		if (m_status != WebSocketStatus::Connected)
			throw std::runtime_error("WebSocket is not connected to receive data");

		m_currentResult.Status = WebSocketReadResultStatus::Initiated;
		m_currentResult.Data.clear();
		m_currentResult.TotalBytesRead = 0;
		m_currentResult.Complete.Reset();
		Receive(m_currentResult);
		/*return std::async(std::launch::async, [this] {
			this->Receive(this->m_currentResult);
			this->m_currentResult.Complete.WaitOnEvent(INFINITE, true);
			return this->m_currentResult;
		});*/
		return std::async(std::launch::async, AsyncReceive, this);
	}

	WebSocketReadResult& AsyncWebSocket::Receive()
	{
		Async::CriticalSectionLock cs(m_cs);
		if (m_status != WebSocketStatus::Connected)
			throw std::runtime_error("WebSocket is not connected to receive data");
		if (m_currentResult.Status == WebSocketReadResultStatus::Initiated)
			throw std::runtime_error("A read operation is already in progress");

		m_currentResult.Status = WebSocketReadResultStatus::Initiated;
		m_currentResult.Data.clear();
		m_currentResult.TotalBytesRead = 0;
		m_currentResult.Complete.Reset();
		return Receive(m_currentResult);
	}

	WebSocketReadResult& AsyncWebSocket::Receive(WebSocketReadResult& receiveBuffer)
	{
		//Async::CriticalSectionLock cs(m_cs);
		if (m_status != WebSocketStatus::Connected)
			throw std::runtime_error("WebSocket is not connected to receive data");

		char* currentBufferPointer = nullptr;
		if (receiveBuffer.Status == WebSocketReadResultStatus::PartialRead)
		{
			const size_t oldSize = receiveBuffer.Data.size();
			receiveBuffer.Data.resize(receiveBuffer.Data.size()+m_bufferBlockSize);
			currentBufferPointer = &receiveBuffer.Data[0] + oldSize;
		}
		else if (receiveBuffer.Status == WebSocketReadResultStatus::Initiated)
		{
			receiveBuffer.Data.resize(m_bufferBlockSize);
			currentBufferPointer = &receiveBuffer.Data[0];
		}
		else
		{
			throw std::runtime_error(__FUNCSIG__ ": unknown status " + std::to_string((DWORD)receiveBuffer.Status));
		}

		// This can potentially block
		const DWORD statusCode = WinHttpWebSocketReceive(
			m_winHttpWebSocket.Get(),
			currentBufferPointer,
			m_bufferBlockSize,
			nullptr,
			nullptr
		);
		// If the server terminates the connection, 12030 will returned.
		if (statusCode != ERROR_SUCCESS)
		{
			m_status = WebSocketStatus::Error;
			throw Error::Win32Error("Connection error when receiving websocket data", statusCode);
		}
		return receiveBuffer;
	}
	void AsyncWebSocket::CloseSocket()
	{
		if (m_winHttpWebSocket != nullptr && m_status == WebSocketStatus::Connected)
		{
			const DWORD success = WinHttpWebSocketClose(
				m_winHttpWebSocket.Get(),
				WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS,
				nullptr,
				0
			);
			if (success != ERROR_SUCCESS)
			{
				m_status = WebSocketStatus::Error;
				throw Error::Win32Error("WinHttpWebSocketClose() failed", success);
			}
			m_status = WebSocketStatus::Closing;
		}
	}

	void AsyncWebSocket::Release()
	{
		m_winHttpWebSocket.Close();
		m_winHttpSession.Close();
		m_winHttpConnection.Close();
	}

	WebSocketStatus AsyncWebSocket::GetStatus() const noexcept
	{
		return m_status;
	}

	const ConnectionResult& AsyncWebSocket::InternalConnect(const std::wstring& path)
	{
		if (m_status != WebSocketStatus::NotInitialised)
			throw std::runtime_error(__FUNCSIG__ ": WebSocket needs to be in NotInitialised state to connect");

		try
		{
			// Synchronous function
			m_winHttpSession = WinHttpOpen(
				m_settings.UserAgent.c_str(),
				WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
				WINHTTP_NO_PROXY_NAME,
				WINHTTP_NO_PROXY_BYPASS,
				WINHTTP_FLAG_ASYNC
			);
			if (m_winHttpSession == nullptr)
			{
				throw Error::Win32Error(
					__FUNCSIG__ ": WinHttpOpen() failed to open the WinHttp session",
					GetLastError()
				);
			}

			// Synchronous function
			m_winHttpConnection = WinHttpConnect(
				m_winHttpSession.Get(),
				m_settings.Server.c_str(),
				m_settings.Port,
				0
			);
			if (m_winHttpConnection == nullptr)
				throw Error::Win32Error(
					__FUNCSIG__ ": WinHttpConnect() failed",
					GetLastError()
				);

			WINHTTP_STATUS_CALLBACK callbackStatus = WinHttpSetStatusCallback(
				m_winHttpConnection.Get(),
				StatusCallback,
				WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, 
				0
			);
			if (callbackStatus == WINHTTP_INVALID_STATUS_CALLBACK)
				throw Error::Win32Error(
					__FUNCSIG__ ": WinHttpSetStatusCallback() failed when setting callback",
					GetLastError()
				);

			bool succeeded = WinHttpSetOption(
				m_winHttpConnection.Get(),
				WINHTTP_OPTION_CONTEXT_VALUE,
				this,
				sizeof(this)
			);
			if (succeeded == false)
				throw Error::Win32Error(
					__FUNCSIG__ ": WinHttpSetOption() failed when setting context value",
					GetLastError()
				);

			// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpopenrequest
			m_requestHandle = WinHttpOpenRequest(
				m_winHttpConnection.Get(),
				L"GET",
				path.c_str(),
				nullptr,
				nullptr,
				nullptr,
				WINHTTP_FLAG_SECURE
			);
			if (m_requestHandle == nullptr)
				throw Error::Win32Error(
					__FUNCSIG__ ": WinHttpOpenRequest() failed", 
					GetLastError()
				);

			const DWORD_PTR _this = (DWORD_PTR)this;
			succeeded = WinHttpSetOption(
				m_requestHandle.Get(),
				WINHTTP_OPTION_CONTEXT_VALUE,
				(void*)&_this,
				sizeof(DWORD_PTR)
			);
			if (succeeded == false)
				throw Error::Win32Error(
					__FUNCSIG__ ": WinHttpSetOption() failed when setting context value",
					GetLastError()
				);

			bool success = false;
			if (m_settings.IgnoreSslErrors)
			{
				DWORD optionFlags = SECURITY_FLAG_IGNORE_ALL_CERT_ERRORS;
				// https://docs.microsoft.com/en-us/windows/win32/winhttp/option-flags
				// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpsetoption
				success = WinHttpSetOption(
					m_requestHandle.Get(),
					WINHTTP_OPTION_SECURITY_FLAGS,
					&optionFlags,
					sizeof(optionFlags)
				);
				if (success == false)
					throw Error::Win32Error(__FUNCSIG__ ": WinHttpSetOption() failed", GetLastError());
			}

			success = WinHttpSetOption(
				m_requestHandle.Get(),
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
					m_requestHandle.Get(),
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
				m_requestHandle.Get(),
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
			return m_connectionResult;
		}
		catch (const std::exception&)
		{
			CloseSocket();
			m_status = WebSocketStatus::Error;
			throw;
		}
	}

	void AsyncWebSocket::Move(AsyncWebSocketSettings& other) noexcept
	{

	}
}