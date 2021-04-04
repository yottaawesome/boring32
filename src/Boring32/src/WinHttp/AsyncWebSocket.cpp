#include "pch.hpp"
#include "include/Strings/Strings.hpp"
#include "include/Error/Error.hpp"
#include "include/WinHttp/WebSockets/AsyncWebSocket.hpp"

namespace Boring32::WinHttp::WebSockets
{
	void AsyncWebSocket::StatusCallback(
		HINTERNET hInternet,
		DWORD_PTR dwContext,
		DWORD dwInternetStatus,
		LPVOID lpvStatusInformation,
		DWORD dwStatusInformationLength
	)
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpsetstatuscallback
		switch (dwInternetStatus)
		{
			case WINHTTP_CALLBACK_STATUS_RESOLVING_NAME:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_RESOLVING_NAME" << std::endl;
				break;
			}

			case WINHTTP_CALLBACK_STATUS_NAME_RESOLVED:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_NAME_RESOLVED" << std::endl;
				break;
			}

			case WINHTTP_CALLBACK_STATUS_CONNECTING_TO_SERVER:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_CONNECTING_TO_SERVER" << std::endl;
				break;
			}

			case WINHTTP_CALLBACK_STATUS_CONNECTED_TO_SERVER:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_CONNECTED_TO_SERVER" << std::endl;
				break;
			}

			case WINHTTP_CALLBACK_STATUS_SENDING_REQUEST:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_SENDING_REQUEST" << std::endl;
				break;
			}
	
			case WINHTTP_CALLBACK_STATUS_REQUEST_SENT:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_REQUEST_SENT" << std::endl;
				bool success = WinHttpReceiveResponse(hInternet, 0);
				if (success == false)
				{
					try
					{
						throw Error::Win32Error(
							__FUNCSIG__ ": WinHttpReceiveResponse() failed on initial connection",
							GetLastError()
						);
					}
					catch (const std::exception& ex)
					{
						std::wcout << ex.what() << std::endl;
					}
				}
					
				break;
			}

			case WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE" << std::endl;
				break;
			}

			case WINHTTP_CALLBACK_STATUS_RESPONSE_RECEIVED:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_RESPONSE_RECEIVED" << std::endl;
				
				break;
			}

			case WINHTTP_CALLBACK_STATUS_CLOSING_CONNECTION:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_CLOSING_CONNECTION" << std::endl;
				break;
			}

			case WINHTTP_CALLBACK_STATUS_CONNECTION_CLOSED:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_CONNECTION_CLOSED" << std::endl;
				break;
			}

			case WINHTTP_CALLBACK_STATUS_HANDLE_CREATED:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_HANDLE_CREATED" << std::endl;
				break;
			}

			case WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING" << std::endl;
				break;
			}

			case WINHTTP_CALLBACK_STATUS_DETECTING_PROXY:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_DETECTING_PROXY" << std::endl;
				break;
			}

			case WINHTTP_CALLBACK_STATUS_REDIRECT:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_REDIRECT" << std::endl;
				break;
			}

			case WINHTTP_CALLBACK_STATUS_INTERMEDIATE_RESPONSE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_INTERMEDIATE_RESPONSE" << std::endl;
				break;
			}

			case WINHTTP_CALLBACK_STATUS_SECURE_FAILURE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_SECURE_FAILURE" << std::endl;
				break;
			}

			case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE" << std::endl;
				break;
			}

			case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE" << std::endl;
				try
				{
					DWORD statusCode = 0;
					DWORD statusCodeSize = sizeof(statusCode);
					// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpqueryheaders
					bool success = WinHttpQueryHeaders(
						hInternet,
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

					AsyncWebSocket* socket = (AsyncWebSocket*)dwContext;
					socket->m_winHttpWebSocket = WinHttpWebSocketCompleteUpgrade(
						socket->m_requestHandle.Get(), 
						0
					);
					if (socket->m_winHttpWebSocket == nullptr)
						throw Error::Win32Error(
							__FUNCSIG__ ": WinHttpWebSocketCompleteUpgrade() failed", 
							GetLastError()
						);

					bool succeeded = WinHttpSetOption(
						socket->m_winHttpWebSocket.Get(),
						WINHTTP_OPTION_CONTEXT_VALUE,
						(void*)&dwContext,
						sizeof(DWORD_PTR)
					);
					if (succeeded == false)
						throw Error::Win32Error(
							__FUNCSIG__ ": WinHttpSetOption() failed when setting context value",
							GetLastError()
						);

					socket->m_status = WebSocketStatus::Connected;
					socket->m_requestHandle = nullptr;
				}
				catch (const std::exception& ex)
				{
					std::wcout << ex.what() << std::endl;
				}
				break;
			}

			case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE" << std::endl;
				break;
			}

			case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_READ_COMPLETE" << std::endl;
				break;
			}

			case WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE" << std::endl;
				break;
			}

			case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_REQUEST_ERROR" << std::endl;
				break;
			}

			case WINHTTP_CALLBACK_STATUS_GETPROXYFORURL_COMPLETE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_GETPROXYFORURL_COMPLETE" << std::endl;
				break;
			}
			case WINHTTP_CALLBACK_STATUS_CLOSE_COMPLETE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_CLOSE_COMPLETE" << std::endl;
				AsyncWebSocket* socket = (AsyncWebSocket*)dwContext;
				socket->Release();
				socket->m_status = WebSocketStatus::Closed;

				break;
			}
			case WINHTTP_CALLBACK_STATUS_SHUTDOWN_COMPLETE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_SHUTDOWN_COMPLETE" << std::endl;
				
				break;
			}
			case WINHTTP_CALLBACK_STATUS_SETTINGS_WRITE_COMPLETE:
			{
				std::wcout << L"" << std::endl;
				break;
			}
			case WINHTTP_CALLBACK_STATUS_SETTINGS_READ_COMPLETE:
			{
				std::wcout << L"" << std::endl;
				break;
			}

			default:
			{
				std::wcout << L"Default" << std::endl;
			}
		}
	}

	AsyncWebSocket::~AsyncWebSocket()
	{
		if (m_status == WebSocketStatus::Connected)
			CloseSocket();
		while (m_status == WebSocketStatus::Closing)
			Sleep(100);
	}

	AsyncWebSocket::AsyncWebSocket(const AsyncWebSocketSettings& settings)
	:	m_settings(settings),
		m_status(WebSocketStatus::NotInitialised)
	{ }

	const AsyncWebSocketSettings& AsyncWebSocket::GetSettings()
	{
		return m_settings;
	}

	void AsyncWebSocket::Connect(const std::wstring& path)
	{
		InternalConnect(path);
	}

	void AsyncWebSocket::Connect()
	{
		InternalConnect(L"");
	}

	void AsyncWebSocket::SendString(const std::string& msg)
	{

	}

	void AsyncWebSocket::SendBuffer(const std::vector<char>& buffer)
	{

	}

	bool AsyncWebSocket::Receive(std::vector<char>& buffer)
	{
		return true;
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
			//if (success != ERROR_SUCCESS)
				//throw Error::Win32Error("WinHttpWebSocketClose() failed", success);
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

	void AsyncWebSocket::InternalConnect(const std::wstring& path)
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

			std::wcout << L"this: " << this <<std::endl;
			DWORD_PTR _this = (DWORD_PTR)this;
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