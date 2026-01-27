export module boring32:winhttp.asyncwebsocket;
import std;
import :win32;
import :error;
import :async;
import :winhttp.winhttphandle;
import :winhttp.enums;
import :winhttp.winhttphandle;
import :winhttp.asyncwebsocketsettings;

constexpr unsigned long BufferBlockSize = 1024;

export namespace Boring32::WinHttp::WebSockets
{
	enum class ReadResultStatus : Win32::DWORD
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
		Win32::DWORD TotalBytesRead = 0;
		std::vector<char> Data;
		Async::ManualResetEvent Complete{ false, false };
	};

	enum class WriteResultStatus : Win32::DWORD
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
		Async::ManualResetEvent Complete{ false,false };
	};

	struct ConnectionResult
	{
		ConnectionResult() = default;
		ConnectionResult(const ConnectionResult&) = delete;
		ConnectionResult& operator=(const ConnectionResult&) = delete;
		ConnectionResult& operator=(ConnectionResult&& other) noexcept = default;

		bool IsConnected = false;
		Async::ManualResetEvent Complete{ false, false };
	};

	struct AsyncWebSocket final
	{
		~AsyncWebSocket()
		{
			if (m_status == WebSocketStatus::Connected) try
			{
				CloseSocket();
				int i = 0;
				while (i < 5 && m_status == WebSocketStatus::Closing)
				{
					Win32::Sleep(100);
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
			std::ignore = m_connectionResult.Complete.Signal(std::nothrow);
			std::ignore = m_readResult.Complete.Signal(std::nothrow);
			std::ignore = m_writeResult.Complete.Signal(std::nothrow);
		}

		AsyncWebSocket(const AsyncWebSocketSettings& settings)
			: m_settings(settings),
			m_status(WebSocketStatus::NotInitialised),
			m_writeResult{},
			m_readResult{},
			m_connectionResult{}
		{ }
		
		const AsyncWebSocketSettings& GetSettings()
		{
			return m_settings;
		}

		const ConnectionResult& Connect()
		{
			return InternalConnect(L"");
		}

		const ConnectionResult& Connect(const std::wstring& path)
		{
			return InternalConnect(path);
		}

		const ConnectionResult& GetConnectionStatus() const
		{
			return m_connectionResult;
		}

		const WriteResult& SendString(const std::string& msg)
		{
			if (m_status != WebSocketStatus::Connected)
				throw Error::Boring32Error("WebSocket is not connected to send data");
			if (m_writeResult.Status != WriteResultStatus::NotInitiated && m_writeResult.Status != WriteResultStatus::Finished)
				throw Error::Boring32Error("A write result is either pending or in error");

			m_writeResult.Status = WriteResultStatus::Initiated;
			m_writeResult.Complete.Reset();
			const Win32::DWORD statusCode = Win32::WinHttp::WinHttpWebSocketSend(
				m_winHttpWebSocket.Get(),
				Win32::WinHttp::WINHTTP_WEB_SOCKET_BUFFER_TYPE::WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
				reinterpret_cast<void*>(const_cast<char*>(&msg[0])),
				static_cast<Win32::DWORD>(msg.size() * sizeof(char))
			);
			if (statusCode != Win32::ErrorCodes::Success)
			{
				m_status = WebSocketStatus::Error;
				throw Error::Win32Error(statusCode, "WinHttpWebSocketSend() failed");
			}
			return m_writeResult;
		}

		const WriteResult& SendBuffer(const std::vector<std::byte>& buffer)
		{
			if (m_status != WebSocketStatus::Connected)
				throw Error::Boring32Error("WebSocket is not connected to send data");

			m_writeResult.Status = WriteResultStatus::Initiated;
			m_writeResult.Complete.Reset();
			const Win32::DWORD statusCode = WinHttpWebSocketSend(
				m_winHttpWebSocket.Get(),
				Win32::WinHttp::WINHTTP_WEB_SOCKET_BUFFER_TYPE::WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE,
				reinterpret_cast<Win32::PVOID>(const_cast<std::byte*>(&buffer[0])),
				static_cast<Win32::DWORD>(buffer.size() * sizeof(char))
			);
			m_writeResult.Status = WriteResultStatus::Initiated;
			if (statusCode != Win32::ErrorCodes::Success)
			{
				m_status = WebSocketStatus::Error;
				m_writeResult.Status = WriteResultStatus::Error;
				throw Error::Win32Error(statusCode, "WinHttpWebSocketSend() failed");
			}
			return m_writeResult;
		}

		const AsyncReadResult& Receive()
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

		void CloseSocket()
		{
			if (not m_winHttpWebSocket and m_status == WebSocketStatus::Connected)
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
					throw Error::Win32Error(success, "WinHttpWebSocketClose() failed");
				}
				m_status = WebSocketStatus::Closing;
			}
			else
			{
				m_status = WebSocketStatus::Closed;
			}
		}

		void Release()
		{
			m_winHttpWebSocket.Close();
			m_winHttpSession.Close();
			m_winHttpConnection.Close();
		}

		WebSocketStatus GetStatus() const noexcept
		{
			return m_status;
		}

		const AsyncReadResult& GetCurrentRead()
		{
			return m_readResult;
		}

	//std::shared_future<WebSocketReadResult> Receive2();
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
		const ConnectionResult& InternalConnect(const std::wstring& path)
		{
			if (m_status != WebSocketStatus::NotInitialised)
				throw Error::Boring32Error("WebSocket needs to be in NotInitialised state to connect");

			try
			{
				// Synchronous function
				m_winHttpSession = Win32::WinHttp::WinHttpOpen(
					m_settings.UserAgent.c_str(),
					Win32::WinHttp::AccessTypeAutomaticProxy,
					(Win32::LPCWSTR)Win32::WinHttp::NoProxyName,
					(Win32::LPCWSTR)Win32::WinHttp::NoProxyBypass,
					Win32::WinHttp::FlagAsync
				);
				if (not m_winHttpSession)
					throw Error::Win32Error(Win32::GetLastError(), "WinHttpOpen() failed to open the WinHttp session");

				// Synchronous function
				m_winHttpConnection = Win32::WinHttp::WinHttpConnect(
					m_winHttpSession.Get(),
					m_settings.Server.c_str(),
					m_settings.Port,
					0
				);
				if (not m_winHttpConnection)
					throw Error::Win32Error(Win32::GetLastError(), "WinHttpConnect() failed");

				Win32::WinHttp::WINHTTP_STATUS_CALLBACK callbackStatus = Win32::WinHttp::WinHttpSetStatusCallback(
					m_winHttpConnection.Get(),
					StatusCallback,
					Win32::WinHttp::CallbackFlagAllNotifications,
					0
				);
				if (callbackStatus == Win32::WinHttp::InvalidStatusCallback)
					throw Error::Win32Error(Win32::GetLastError(), "WinHttpSetStatusCallback() failed when setting callback");

				Win32::DWORD_PTR _this = reinterpret_cast<Win32::DWORD_PTR>(this);
				bool succeeded = Win32::WinHttp::WinHttpSetOption(
					m_winHttpConnection.Get(),
					Win32::WinHttp::Options::ContextValue,
					reinterpret_cast<void*>(&_this),
					sizeof(Win32::DWORD_PTR)
				);
				if (not succeeded)
					throw Error::Win32Error(Win32::GetLastError(), "WinHttpSetOption() failed when setting context value");

				// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpopenrequest
				m_requestHandle = Win32::WinHttp::WinHttpOpenRequest(
					m_winHttpConnection.Get(),
					L"GET",
					path.c_str(),
					nullptr,
					nullptr,
					nullptr,
					Win32::WinHttp::FlagSecure
				);
				if (not m_requestHandle)
					throw Error::Win32Error(Win32::GetLastError(), "WinHttpOpenRequest() failed");

				succeeded = Win32::WinHttp::WinHttpSetOption(
					m_requestHandle.Get(),
					Win32::WinHttp::Options::ContextValue,
					reinterpret_cast<void*>(&_this),
					sizeof(Win32::DWORD_PTR)
				);
				if (not succeeded)
					throw Error::Win32Error(Win32::GetLastError(), "WinHttpSetOption() failed when setting context value");

				if (m_settings.IgnoreSslErrors)
				{
					Win32::DWORD optionFlags = Win32::WinHttp::SecurityFlagIgnoreAllCertErrors;
					// https://docs.microsoft.com/en-us/windows/win32/winhttp/option-flags
					// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpsetoption
					succeeded = Win32::WinHttp::WinHttpSetOption(
						m_requestHandle.Get(),
						Win32::WinHttp::Options::SecurityFlags,
						&optionFlags,
						sizeof(optionFlags)
					);
					if (not succeeded)
						throw Error::Win32Error(Win32::GetLastError(), "WinHttpSetOption() failed");
				}

				succeeded = Win32::WinHttp::WinHttpSetOption(
					m_requestHandle.Get(),
					Win32::WinHttp::Options::UpgradeToWebSocket,
					nullptr,
					0
				);
				if (not succeeded)
					throw Error::Win32Error(Win32::GetLastError(), "WinHttpSetOption() failed");

				if (m_settings.ClientCert.GetCert())
				{
					// If so, we need to set the certificate option, and retry the request.
					bool setCertOption = Win32::WinHttp::WinHttpSetOption(
						m_requestHandle.Get(),
						Win32::WinHttp::Options::ClientCertContext,
						(void*)m_settings.ClientCert.GetCert(),
						sizeof(Win32::CERT_CONTEXT)
					);
					if (not setCertOption)
						throw Error::Win32Error(Win32::GetLastError(), "WinHttpSetOption() failed for client certificate");
				}

				const wchar_t* connectionHeaders = m_settings.ConnectionHeaders.empty()
					? (wchar_t*)Win32::WinHttp::NoAdditionalHeaders
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
				if (not succeeded)
					throw Error::Win32Error(Win32::GetLastError(), "WinHttpSendRequest() failed on initial request");
						
				return m_connectionResult;
			}
			catch (const std::exception&)
			{
				CloseSocket();
				m_status = WebSocketStatus::Error;
				throw;
			}
		}

		const AsyncReadResult& Receive(AsyncReadResult& receiveBuffer)
		{
			//Async::CriticalSectionLock cs(m_cs);
			if (m_status != WebSocketStatus::Connected)
				throw Error::Boring32Error("WebSocket is not connected to receive data");

			char* currentBufferPointer = nullptr;
			if (receiveBuffer.Status == ReadResultStatus::PartialRead)
			{
				const size_t oldSize = receiveBuffer.Data.size();
				receiveBuffer.Data.resize(receiveBuffer.Data.size() + BufferBlockSize);
				currentBufferPointer = &receiveBuffer.Data[0] + oldSize;
			}
			else if (receiveBuffer.Status == ReadResultStatus::Initiated)
			{
				receiveBuffer.Data.resize(BufferBlockSize);
				currentBufferPointer = &receiveBuffer.Data[0];
			}
			else
			{
				throw Error::Boring32Error("unknown status " + std::to_string((DWORD)receiveBuffer.Status));
			}

			// This can potentially block
			Win32::DWORD statusCode = Win32::WinHttp::WinHttpWebSocketReceive(
				m_winHttpWebSocket.Get(),
				currentBufferPointer,
				BufferBlockSize,
				nullptr,
				nullptr
			);
			// If the server terminates the connection, 12030 will returned.
			if (statusCode != Win32::ErrorCodes::Success)
			{
				m_status = WebSocketStatus::Error;
				m_readResult.Complete.Signal();
				m_readResult.Status = ReadResultStatus::Error;
				throw Error::Win32Error(statusCode, "Connection error when receiving websocket data");
			}

			return receiveBuffer;
		}

		void Move(AsyncWebSocket& other) noexcept
		{
			CloseSocket();
			m_settings = std::move(other.m_settings);
			m_winHttpConnection = std::move(other.m_winHttpConnection);
			m_winHttpSession = std::move(other.m_winHttpSession);
			m_winHttpWebSocket = std::move(other.m_winHttpWebSocket);
			m_status = other.m_status;
			m_requestHandle = std::move(other.m_requestHandle);
			//CRITICAL_SECTION m_cs;
			m_readResult = std::move(other.m_readResult);
			m_connectionResult = std::move(other.m_connectionResult);
			m_writeResult = std::move(other.m_writeResult);
		}

		void CompleteUpgrade()
		{
			if (not m_requestHandle)
				throw Error::Boring32Error("m_requestHandle is nullptr");

			m_winHttpWebSocket = Win32::WinHttp::WinHttpWebSocketCompleteUpgrade(m_requestHandle.Get(), 0);
			if (not m_winHttpWebSocket)
				throw Error::Win32Error(Win32::GetLastError(), "WinHttpWebSocketCompleteUpgrade() failed");

			Win32::DWORD_PTR dwThis = reinterpret_cast<Win32::DWORD_PTR>(this);
			const bool succeeded = Win32::WinHttp::WinHttpSetOption(
				m_winHttpWebSocket.Get(),
				Win32::WinHttp::Options::ContextValue,
				reinterpret_cast<void*>(&dwThis),
				sizeof(Win32::DWORD_PTR)
			);
			if (not succeeded)
				throw Error::Win32Error(Win32::GetLastError(), "WinHttpSetOption() failed when setting context value");

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

		AsyncWebSocketSettings m_settings;
		WinHttpHandle m_winHttpConnection;
		WinHttpHandle m_winHttpSession;
		WinHttpHandle m_winHttpWebSocket;
		WebSocketStatus m_status;
		WinHttpHandle m_requestHandle;
		//CRITICAL_SECTION m_cs;
		AsyncReadResult m_readResult;
		ConnectionResult m_connectionResult;
		WriteResult m_writeResult;
	};
}
