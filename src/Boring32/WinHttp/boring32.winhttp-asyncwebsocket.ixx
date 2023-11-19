export module boring32.winhttp:asyncwebsocket;
import <vector>;
import <string>;
import <future>;
import <memory>;
import <algorithm>;
import <iostream>;
import boring32.async;
import boring32.error;
import boring32.win32;
import :winhttphandle;
import :websocketstatus;
import :winhttphandle;
import :asyncwebsocketsettings;

export namespace Boring32::WinHttp::WebSockets
{
	enum class ReadResultStatus : DWORD
	{
		NotInitiated,
		Initiated,
		PartialRead,
		Finished,
		Error
	};

	struct AsyncReadResult final
	{
		AsyncReadResult() = default;
		AsyncReadResult(const AsyncReadResult&) = delete;
		AsyncReadResult& operator=(const AsyncReadResult&) = delete;
		AsyncReadResult& operator=(AsyncReadResult&& other) noexcept = default;

		ReadResultStatus Status = ReadResultStatus::NotInitiated;
		DWORD TotalBytesRead = 0;
		std::vector<char> Data;
		Async::Event Complete{ false, true, false };
	};

	enum class WriteResultStatus : DWORD
	{
		NotInitiated,
		Initiated,
		Error,
		Finished
	};

	struct WriteResult final
	{
		WriteResult() = default;
		WriteResult(const WriteResult&) = delete;
		WriteResult& operator=(const WriteResult&) = delete;
		WriteResult& operator=(WriteResult&& other) noexcept = default;

		WriteResultStatus Status = WriteResultStatus::NotInitiated;
		Async::Event Complete{ false,true,false };
	};

	struct ConnectionResult
	{
		ConnectionResult() = default;
		ConnectionResult(const ConnectionResult&) = delete;
		ConnectionResult& operator=(const ConnectionResult&) = delete;
		ConnectionResult& operator=(ConnectionResult&& other) noexcept = default;

		bool IsConnected = false;
		Async::Event Complete{ false, true, false };
	};

	class AsyncWebSocket
	{
		public:
			virtual ~AsyncWebSocket()
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
					std::wcerr << ex.what() << std::endl;
				}

				// Release any waiting threads. However, all threads that use
				// this socket should be shut down prior to the socket going
				// out of scope
				m_connectionResult.Complete.Signal(std::nothrow);
				m_readResult.Complete.Signal(std::nothrow);
				m_writeResult.Complete.Signal(std::nothrow);
			}

			AsyncWebSocket(const AsyncWebSocketSettings& settings)
				: m_settings(settings),
				m_status(WebSocketStatus::NotInitialised),
				m_writeResult{},
				m_readResult{},
				m_connectionResult{}
			{ }
		
		public:
			virtual const AsyncWebSocketSettings& GetSettings()
			{
				return m_settings;
			}

			virtual const ConnectionResult& Connect()
			{
				return InternalConnect(L"");
			}

			virtual const ConnectionResult& Connect(const std::wstring& path)
			{
				return InternalConnect(path);
			}

			virtual const ConnectionResult& GetConnectionStatus() const
			{
				return m_connectionResult;
			}

			virtual const WriteResult& SendString(const std::string& msg)
			{
				if (m_status != WebSocketStatus::Connected)
					throw Error::Boring32Error("WebSocket is not connected to send data");
				if (m_writeResult.Status != WriteResultStatus::NotInitiated && m_writeResult.Status != WriteResultStatus::Finished)
					throw Error::Boring32Error("A write result is either pending or in error");

				m_writeResult.Status = WriteResultStatus::Initiated;
				m_writeResult.Complete.Reset();
				const DWORD statusCode = WinHttpWebSocketSend(
					m_winHttpWebSocket.Get(),
					WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
					reinterpret_cast<void*>(const_cast<char*>(&msg[0])),
					static_cast<DWORD>(msg.size() * sizeof(char))
				);
				if (statusCode != Win32::ErrorCodes::Success)
				{
					m_status = WebSocketStatus::Error;
					throw Error::Win32Error("WinHttpWebSocketSend() failed", statusCode);
				}
				return m_writeResult;
			}

			virtual const WriteResult& SendBuffer(const std::vector<std::byte>& buffer)
			{
				if (m_status != WebSocketStatus::Connected)
					throw Error::Boring32Error("WebSocket is not connected to send data");

				m_writeResult.Status = WriteResultStatus::Initiated;
				m_writeResult.Complete.Reset();
				const DWORD statusCode = WinHttpWebSocketSend(
					m_winHttpWebSocket.Get(),
					WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE,
					reinterpret_cast<PVOID>(const_cast<std::byte*>(&buffer[0])),
					static_cast<DWORD>(buffer.size() * sizeof(char))
				);
				m_writeResult.Status = WriteResultStatus::Initiated;
				if (statusCode != Win32::ErrorCodes::Success)
				{
					m_status = WebSocketStatus::Error;
					m_writeResult.Status = WriteResultStatus::Error;
					throw Error::Win32Error("WinHttpWebSocketSend() failed", statusCode);
				}
				return m_writeResult;
			}

			virtual const AsyncReadResult& Receive()
			{
				//Async::CriticalSectionLock cs(m_cs);
				if (m_status != WebSocketStatus::Connected)
					throw Error::Boring32Error("WebSocket is not connected to receive data");
				if (m_readResult.Status == ReadResultStatus::Initiated)
					throw Error::Boring32Error("A read operation is already in progress");

				m_readResult.Status = ReadResultStatus::Initiated;
				m_readResult.Data.clear();
				m_readResult.TotalBytesRead = 0;
				m_readResult.Complete.Reset();
				Receive(m_readResult);
				return m_readResult;
			}

			virtual void CloseSocket()
			{
				if (m_winHttpWebSocket != nullptr && m_status == WebSocketStatus::Connected)
				{
					const Win32::DWORD success = Win32::WinHttp::WinHttpWebSocketClose(
						m_winHttpWebSocket.Get(),
						Win32::WinHttp::WINHTTP_WEB_SOCKET_CLOSE_STATUS::WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS,
						nullptr,
						0
					);
					if (success != Win32::ErrorCodes::Success)
					{
						m_status = WebSocketStatus::Error;
						throw Error::Win32Error("WinHttpWebSocketClose() failed", success);
					}
					m_status = WebSocketStatus::Closing;
				}
				else
				{
					m_status = WebSocketStatus::Closed;
				}
			}

			virtual void Release()
			{
				m_winHttpWebSocket.Close();
				m_winHttpSession.Close();
				m_winHttpConnection.Close();
			}

			virtual WebSocketStatus GetStatus() const noexcept
			{
				return m_status;
			}

			virtual const AsyncReadResult& GetCurrentRead()
			{
				return m_readResult;
			}

			//virtual std::shared_future<WebSocketReadResult> Receive2();
			/*WebSocketReadResult AsyncWebSocket::AsyncReceive(AsyncWebSocket* socket)
	{
		if (socket->m_status != WebSocketStatus::Connected)
			throw std::runtime_error("WebSocket is not connected to receive data");
		socket->m_currentReadResult.Complete.WaitOnEvent(INFINITE, true);
		return socket->m_currentReadResult;
	}*/

	//std::shared_future<WebSocketReadResult> AsyncWebSocket::Receive2()
	//{
	//	if (m_status != WebSocketStatus::Connected)
	//		throw std::runtime_error("WebSocket is not connected to receive data");

	//	m_currentReadResult.Status = WebSocketReadResultStatus::Initiated;
	//	m_currentReadResult.Data.clear();
	//	m_currentReadResult.TotalBytesRead = 0;
	//	m_currentReadResult.Complete.Reset();
	//	Receive(m_currentReadResult);
	//	/*return std::async(std::launch::async, [this] {
	//		this->Receive(this->m_currentResult);
	//		this->m_currentResult.Complete.WaitOnEvent(INFINITE, true);
	//		return this->m_currentResult;
	//	});*/
	//	return std::async(std::launch::async, AsyncReceive, this);
	//}

		protected:
			virtual const ConnectionResult& InternalConnect(const std::wstring& path)
			{
				if (m_status != WebSocketStatus::NotInitialised)
					throw Error::Boring32Error("WebSocket needs to be in NotInitialised state to connect");

				try
				{
					// Synchronous function
					m_winHttpSession = Win32::WinHttp::WinHttpOpen(
						m_settings.UserAgent.c_str(),
						Win32::WinHttp::_WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
						(Win32::LPCWSTR)Win32::WinHttp::_WINHTTP_NO_PROXY_NAME,
						(Win32::LPCWSTR)Win32::WinHttp::_WINHTTP_NO_PROXY_BYPASS,
						Win32::WinHttp::_WINHTTP_FLAG_ASYNC
					);
					if (!m_winHttpSession)
					{
						const auto lastError = Win32::GetLastError();
						throw Error::Win32Error(
							"WinHttpOpen() failed to open the WinHttp session",
							lastError
						);
					}

					// Synchronous function
					m_winHttpConnection = Win32::WinHttp::WinHttpConnect(
						m_winHttpSession.Get(),
						m_settings.Server.c_str(),
						m_settings.Port,
						0
					);
					if (!m_winHttpConnection)
					{
						const auto lastError = Win32::GetLastError();
						throw Error::Win32Error(
							"WinHttpConnect() failed",
							lastError
						);
					}

					Win32::WinHttp::WINHTTP_STATUS_CALLBACK callbackStatus = Win32::WinHttp::WinHttpSetStatusCallback(
						m_winHttpConnection.Get(),
						StatusCallback,
						Win32::WinHttp::_WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS,
						0
					);
					if (callbackStatus == Win32::WinHttp::_WINHTTP_INVALID_STATUS_CALLBACK)
					{
						const auto lastError = Win32::GetLastError();
						throw Error::Win32Error(
							"WinHttpSetStatusCallback() failed when setting callback",
							lastError
						);
					}

					Win32::DWORD_PTR _this = reinterpret_cast<Win32::DWORD_PTR>(this);
					bool succeeded = Win32::WinHttp::WinHttpSetOption(
						m_winHttpConnection.Get(),
						Win32::WinHttp::_WINHTTP_OPTION_CONTEXT_VALUE,
						reinterpret_cast<void*>(&_this),
						sizeof(Win32::DWORD_PTR)
					);
					if (!succeeded)
					{
						const auto lastError = Win32::GetLastError();
						throw Error::Win32Error(
							"WinHttpSetOption() failed when setting context value",
							lastError
						);
					}

					// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpopenrequest
					m_requestHandle = Win32::WinHttp::WinHttpOpenRequest(
						m_winHttpConnection.Get(),
						L"GET",
						path.c_str(),
						nullptr,
						nullptr,
						nullptr,
						Win32::WinHttp::_WINHTTP_FLAG_SECURE
					);
					if (!m_requestHandle)
					{
						const auto lastError = Win32::GetLastError();
						throw Error::Win32Error(
							"WinHttpOpenRequest() failed",
							lastError
						);
					}

					succeeded = Win32::WinHttp::WinHttpSetOption(
						m_requestHandle.Get(),
						Win32::WinHttp::_WINHTTP_OPTION_CONTEXT_VALUE,
						reinterpret_cast<void*>(&_this),
						sizeof(Win32::DWORD_PTR)
					);
					if (!succeeded)
					{
						const auto lastError = Win32::GetLastError();
						throw Error::Win32Error(
							"WinHttpSetOption() failed when setting context value",
							lastError
						);
					}

					if (m_settings.IgnoreSslErrors)
					{
						Win32::DWORD optionFlags = Win32::WinHttp::_SECURITY_FLAG_IGNORE_ALL_CERT_ERRORS;
						// https://docs.microsoft.com/en-us/windows/win32/winhttp/option-flags
						// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpsetoption
						succeeded = Win32::WinHttp::WinHttpSetOption(
							m_requestHandle.Get(),
							Win32::WinHttp::_WINHTTP_OPTION_SECURITY_FLAGS,
							&optionFlags,
							sizeof(optionFlags)
						);
						if (!succeeded)
						{
							const auto lastError = Win32::GetLastError();
							throw Error::Win32Error("WinHttpSetOption() failed", lastError);
						}
					}

					succeeded = Win32::WinHttp::WinHttpSetOption(
						m_requestHandle.Get(),
						Win32::WinHttp::_WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET,
						nullptr,
						0
					);
					if (!succeeded)
					{
						const auto lastError = Win32::GetLastError();
						throw Error::Win32Error("WinHttpSetOption() failed", lastError);
					}

					if (m_settings.ClientCert.GetCert())
					{
						// If so, we need to set the certificate option, and retry the request.
						const bool setCertOption = Win32::WinHttp::WinHttpSetOption(
							m_requestHandle.Get(),
							Win32::WinHttp::_WINHTTP_OPTION_CLIENT_CERT_CONTEXT,
							(void*)m_settings.ClientCert.GetCert(),
							sizeof(Win32::CERT_CONTEXT)
						);
						if (!setCertOption)
						{
							const auto lastError = Win32::GetLastError();
							throw Error::Win32Error("WinHttpSetOption() failed for client certificate", lastError);
						}
					}

					const wchar_t* connectionHeaders = m_settings.ConnectionHeaders.empty()
						? (wchar_t*)Win32::WinHttp::_WINHTTP_NO_ADDITIONAL_HEADERS
						: m_settings.ConnectionHeaders.c_str();
					succeeded = Win32::WinHttp::WinHttpSendRequest(
						m_requestHandle.Get(),
						connectionHeaders,
						-1L,
						nullptr,
						0,
						0,
						0
					);
					if (!succeeded)
					{
						const auto lastError = Win32::GetLastError();
						throw Error::Win32Error(
							"WinHttpSendRequest() failed on initial request",
							lastError
						);
					}
						
					return m_connectionResult;
				}
				catch (const std::exception&)
				{
					CloseSocket();
					m_status = WebSocketStatus::Error;
					throw;
				}
			}

			virtual const AsyncReadResult& Receive(AsyncReadResult& receiveBuffer)
			{
				//Async::CriticalSectionLock cs(m_cs);
				if (m_status != WebSocketStatus::Connected)
					throw Error::Boring32Error("WebSocket is not connected to receive data");

				char* currentBufferPointer = nullptr;
				if (receiveBuffer.Status == ReadResultStatus::PartialRead)
				{
					const size_t oldSize = receiveBuffer.Data.size();
					receiveBuffer.Data.resize(receiveBuffer.Data.size() + m_bufferBlockSize);
					currentBufferPointer = &receiveBuffer.Data[0] + oldSize;
				}
				else if (receiveBuffer.Status == ReadResultStatus::Initiated)
				{
					receiveBuffer.Data.resize(m_bufferBlockSize);
					currentBufferPointer = &receiveBuffer.Data[0];
				}
				else
				{
					throw Error::Boring32Error("unknown status " + std::to_string((DWORD)receiveBuffer.Status));
				}

				// This can potentially block
				const Win32::DWORD statusCode = Win32::WinHttp::WinHttpWebSocketReceive(
					m_winHttpWebSocket.Get(),
					currentBufferPointer,
					m_bufferBlockSize,
					nullptr,
					nullptr
				);
				// If the server terminates the connection, 12030 will returned.
				if (statusCode != Win32::ErrorCodes::Success)
				{
					m_status = WebSocketStatus::Error;
					m_readResult.Complete.Signal();
					m_readResult.Status = ReadResultStatus::Error;
					throw Error::Win32Error("Connection error when receiving websocket data", statusCode);
				}

				return receiveBuffer;
			}

			virtual void Move(AsyncWebSocket& other) noexcept
			{
				CloseSocket();
				m_settings = std::move(other.m_settings);
				m_winHttpConnection = std::move(other.m_winHttpConnection);
				m_winHttpSession = std::move(other.m_winHttpSession);
				m_winHttpWebSocket = std::move(other.m_winHttpWebSocket);
				m_status = other.m_status;
				m_requestHandle = std::move(other.m_requestHandle);
				//CRITICAL_SECTION m_cs;
				m_bufferBlockSize = std::move(other.m_bufferBlockSize);
				m_readResult = std::move(other.m_readResult);
				m_connectionResult = std::move(other.m_connectionResult);
				m_writeResult = std::move(other.m_writeResult);
			}

			virtual void CompleteUpgrade()
			{
				if (!m_requestHandle)
					throw Error::Boring32Error("m_requestHandle is nullptr");

				m_winHttpWebSocket = Win32::WinHttp::WinHttpWebSocketCompleteUpgrade(m_requestHandle.Get(), 0);
				if (!m_winHttpWebSocket)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error(
						"WinHttpWebSocketCompleteUpgrade() failed",
						lastError
					);
				}

				Win32::DWORD_PTR dwThis = reinterpret_cast<Win32::DWORD_PTR>(this);
				const bool succeeded = Win32::WinHttp::WinHttpSetOption(
					m_winHttpWebSocket.Get(),
					Win32::WinHttp::_WINHTTP_OPTION_CONTEXT_VALUE,
					reinterpret_cast<void*>(&dwThis),
					sizeof(Win32::DWORD_PTR)
				);
				if (!succeeded)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error(
						"WinHttpSetOption() failed when setting context value",
						lastError
					);
				}

				m_status = WebSocketStatus::Connected;
				m_connectionResult.IsConnected = true;
				m_connectionResult.Complete.Signal();
				m_requestHandle = nullptr;
			}

			static void StatusCallback(
				Win32::WinHttp::HINTERNET hInternet,
				Win32::DWORD_PTR dwContext,
				Win32::DWORD dwInternetStatus,
				Win32::LPVOID lpvStatusInformation,
				Win32::DWORD dwStatusInformationLength
			);
			//static WebSocketReadResult AsyncReceive(AsyncWebSocket* socket);

		protected:
			AsyncWebSocketSettings m_settings;
			WinHttpHandle m_winHttpConnection;
			WinHttpHandle m_winHttpSession;
			WinHttpHandle m_winHttpWebSocket;
			WebSocketStatus m_status;
			WinHttpHandle m_requestHandle;
			//CRITICAL_SECTION m_cs;
			static Win32::DWORD m_bufferBlockSize;
			AsyncReadResult m_readResult;
			ConnectionResult m_connectionResult;
			WriteResult m_writeResult;
	};
	
	Win32::DWORD AsyncWebSocket::m_bufferBlockSize = 1024;

}
