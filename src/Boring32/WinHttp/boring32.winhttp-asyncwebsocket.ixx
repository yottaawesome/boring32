module;

#include <source_location>;

export module boring32.winhttp:asyncwebsocket;
import <vector>;
import <string>;
import <future>;
import <memory>;
import <algorithm>;
import <stdexcept>;
import <iostream>;
import <win32.hpp>;
import :websocketstatus;
import :asyncwebsocketsettings;
import boring32.async;
import :winhttphandle;
import boring32.error;
import :winhttphandle;

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

	struct AsyncReadResult
	{
		AsyncReadResult() {};
		AsyncReadResult(const AsyncReadResult&) = delete;
		AsyncReadResult& operator=(const AsyncReadResult&) = delete;
		virtual AsyncReadResult& operator=(AsyncReadResult&& other) noexcept
		{
			Status = other.Status;
			TotalBytesRead = other.TotalBytesRead;
			Data = std::move(other.Data);
			Complete = std::move(other.Complete);
			return *this;
		};

		ReadResultStatus Status = ReadResultStatus::NotInitiated;
		DWORD TotalBytesRead = 0;
		std::vector<char> Data;
		Async::Event Complete{ false,true,false };
	};

	enum class WriteResultStatus : DWORD
	{
		NotInitiated,
		Initiated,
		Error,
		Finished
	};

	struct WriteResult
	{
		WriteResult() {};
		WriteResult(const WriteResult&) = delete;
		virtual WriteResult& operator=(const WriteResult&) = delete;
		virtual WriteResult& operator=(WriteResult&& other) noexcept
		{ 
			Status = other.Status; 
			Complete = std::move(other.Complete);
			return *this;
		};

		WriteResultStatus Status = WriteResultStatus::NotInitiated;
		Async::Event Complete{ false,true,false };
	};

	struct ConnectionResult
	{
		ConnectionResult() {};
		ConnectionResult(const ConnectionResult&) = delete;
		ConnectionResult& operator=(const ConnectionResult&) = delete;
		virtual ConnectionResult& operator=(ConnectionResult&& other) noexcept
		{
			IsConnected = other.IsConnected;
			Complete = std::move(other.Complete);
			return *this;
		};

		bool IsConnected=false;
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
					std::wcerr << __FUNCSIG__ ": " << ex.what() << std::endl;
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
					throw std::runtime_error("WebSocket is not connected to send data");
				if (m_writeResult.Status != WriteResultStatus::NotInitiated && m_writeResult.Status != WriteResultStatus::Finished)
					throw std::runtime_error(__FUNCSIG__": a write result is either pending or in error");

				m_writeResult.Status = WriteResultStatus::Initiated;
				m_writeResult.Complete.Reset();
				const DWORD statusCode = WinHttpWebSocketSend(
					m_winHttpWebSocket.Get(),
					WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
					reinterpret_cast<void*>(const_cast<char*>(&msg[0])),
					static_cast<DWORD>(msg.size() * sizeof(char))
				);
				if (statusCode != ERROR_SUCCESS)
				{
					m_status = WebSocketStatus::Error;
					throw Error::Win32Error("WinHttpWebSocketSend() failed", statusCode);
				}
				return m_writeResult;
			}

			virtual const WriteResult& SendBuffer(const std::vector<std::byte>& buffer)
			{
				if (m_status != WebSocketStatus::Connected)
					throw std::runtime_error("WebSocket is not connected to send data");

				m_writeResult.Status = WriteResultStatus::Initiated;
				m_writeResult.Complete.Reset();
				const DWORD statusCode = WinHttpWebSocketSend(
					m_winHttpWebSocket.Get(),
					WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE,
					reinterpret_cast<PVOID>(const_cast<std::byte*>(&buffer[0])),
					static_cast<DWORD>(buffer.size() * sizeof(char))
				);
				m_writeResult.Status = WriteResultStatus::Initiated;
				if (statusCode != ERROR_SUCCESS)
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
					throw std::runtime_error("WebSocket is not connected to receive data");
				if (m_readResult.Status == ReadResultStatus::Initiated)
					throw std::runtime_error("A read operation is already in progress");

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
							"WinHttpOpen() failed to open the WinHttp session",
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
							"WinHttpConnect() failed",
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
							"WinHttpSetStatusCallback() failed when setting callback",
							GetLastError()
						);

					DWORD_PTR _this = reinterpret_cast<DWORD_PTR>(this);
					bool succeeded = WinHttpSetOption(
						m_winHttpConnection.Get(),
						WINHTTP_OPTION_CONTEXT_VALUE,
						reinterpret_cast<void*>(&_this),
						sizeof(DWORD_PTR)
					);
					if (succeeded == false)
						throw Error::Win32Error(
							"WinHttpSetOption() failed when setting context value",
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
							"WinHttpOpenRequest() failed",
							GetLastError()
						);

					succeeded = WinHttpSetOption(
						m_requestHandle.Get(),
						WINHTTP_OPTION_CONTEXT_VALUE,
						reinterpret_cast<void*>(&_this),
						sizeof(DWORD_PTR)
					);
					if (succeeded == false)
						throw Error::Win32Error(
							"WinHttpSetOption() failed when setting context value",
							GetLastError()
						);

					if (m_settings.IgnoreSslErrors)
					{
						DWORD optionFlags = SECURITY_FLAG_IGNORE_ALL_CERT_ERRORS;
						// https://docs.microsoft.com/en-us/windows/win32/winhttp/option-flags
						// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpsetoption
						succeeded = WinHttpSetOption(
							m_requestHandle.Get(),
							WINHTTP_OPTION_SECURITY_FLAGS,
							&optionFlags,
							sizeof(optionFlags)
						);
						if (succeeded == false)
							throw Error::Win32Error("WinHttpSetOption() failed", GetLastError());
					}

					succeeded = WinHttpSetOption(
						m_requestHandle.Get(),
						WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET,
						nullptr,
						0
					);
					if (succeeded == false)
						throw Error::Win32Error("WinHttpSetOption() failed", GetLastError());

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
							throw Error::Win32Error("WinHttpSetOption() failed for client certificate", GetLastError());
					}

					const wchar_t* connectionHeaders = m_settings.ConnectionHeaders.empty()
						? WINHTTP_NO_ADDITIONAL_HEADERS
						: m_settings.ConnectionHeaders.c_str();
					succeeded = WinHttpSendRequest(
						m_requestHandle.Get(),
						connectionHeaders,
						-1L,
						nullptr,
						0,
						0,
						0
					);
					if (succeeded == false)
						throw Error::Win32Error(
							"WinHttpSendRequest() failed on initial request",
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

			virtual const AsyncReadResult& Receive(AsyncReadResult& receiveBuffer)
			{
				//Async::CriticalSectionLock cs(m_cs);
				if (m_status != WebSocketStatus::Connected)
					throw std::runtime_error("WebSocket is not connected to receive data");

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
				if (m_requestHandle == nullptr)
					throw std::runtime_error(__FUNCSIG__": m_requestHandle is nullptr");

				m_winHttpWebSocket = WinHttpWebSocketCompleteUpgrade(m_requestHandle.Get(), 0);
				if (m_winHttpWebSocket == nullptr)
					throw Error::Win32Error(
						"WinHttpWebSocketCompleteUpgrade() failed",
						GetLastError()
					);

				DWORD_PTR dwThis = reinterpret_cast<DWORD_PTR>(this);
				const bool succeeded = WinHttpSetOption(
					m_winHttpWebSocket.Get(),
					WINHTTP_OPTION_CONTEXT_VALUE,
					reinterpret_cast<void*>(&dwThis),
					sizeof(DWORD_PTR)
				);
				if (succeeded == false)
					throw Error::Win32Error(
						"WinHttpSetOption() failed when setting context value",
						GetLastError()
					);

				m_status = WebSocketStatus::Connected;
				m_connectionResult.IsConnected = true;
				m_connectionResult.Complete.Signal();
				m_requestHandle = nullptr;
			}

			static void StatusCallback(
				HINTERNET hInternet,
				DWORD_PTR dwContext,
				DWORD dwInternetStatus,
				LPVOID lpvStatusInformation,
				DWORD dwStatusInformationLength
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
			static DWORD m_bufferBlockSize;
			AsyncReadResult m_readResult;
			ConnectionResult m_connectionResult;
			WriteResult m_writeResult;
	};
	
	DWORD AsyncWebSocket::m_bufferBlockSize = 1024;

}
