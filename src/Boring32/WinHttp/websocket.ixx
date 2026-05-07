export module boring32:winhttp.websocket;
import std;
import :win32;
import :error;
import :util;
import :winhttp.proxyinfo;
import :winhttp.enums;
import :winhttp.websocketsettings;
import :winhttp.winhttphandle;
import :winhttp.session;
import :async;

export namespace Boring32::WinHttp::WebSockets
{
	class WebSocket final
	{
	public:
		struct ReadResult
		{
			std::vector<char> Buffer;
			bool Succeeded = false;
			Async::ManualResetEvent Done{ false, false };
			//std::future<void> Future;
		};

		~WebSocket()
		{
			Close();
		}

		WebSocket() = default;

		WebSocket(const WebSocket&) = delete;
		auto operator=(const WebSocket&) -> WebSocket& = delete;

		WebSocket(WebSocket&& other) noexcept = default;
		auto operator=(WebSocket&& other) noexcept -> WebSocket& = default;

		WebSocket(WebSocketSettings settings)
			: m_settings(std::move(settings)),
			m_winHttpConnection(nullptr),
			m_status(WebSocketStatus::NotInitialised)
		{ }

		auto GetSettings() const noexcept -> WebSocketSettings
		{
			return m_settings;
		}

		void Connect()
		{
			InternalConnect(L"");
		}

		void Connect(const std::wstring& path)
		{
			InternalConnect(path);
		}

		void SendString(const std::string& msg)
		{
			if (m_status != WebSocketStatus::Connected)
				throw Error::Boring32Error{"WebSocket is not connected to send data"};
			auto statusCode = 
				Win32::WinHttp::WinHttpWebSocketSend(
					m_winHttpWebSocket.Get(),
					Win32::WinHttp::WINHTTP_WEB_SOCKET_BUFFER_TYPE::WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
					reinterpret_cast<Win32::PVOID>(const_cast<char*>(&msg[0])),
					static_cast<Win32::DWORD>(msg.size() * sizeof(char))
				);
			if (statusCode != Win32::ErrorCodes::Success)
			{
				m_status = WebSocketStatus::Error;
				throw Error::Win32Error{statusCode, "WinHttpWebSocketSend() failed"};
			}
		}

		void SendBuffer(const std::vector<std::byte>& buffer)
		{
			if (m_status != WebSocketStatus::Connected)
				throw Error::Boring32Error{"WebSocket is not connected to send data"};

			auto statusCode = 
				Win32::WinHttp::WinHttpWebSocketSend(
					m_winHttpWebSocket.Get(),
					Win32::WinHttp::WINHTTP_WEB_SOCKET_BUFFER_TYPE::WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE,
					reinterpret_cast<Win32::PVOID>(const_cast<std::byte*>(&buffer[0])),
					static_cast<Win32::DWORD>(buffer.size() * sizeof(std::byte))
				);
			if (statusCode != Win32::ErrorCodes::Success)
			{
				m_status = WebSocketStatus::Error;
				throw Error::Win32Error{statusCode, "WinHttpWebSocketSend() failed"};
			}
		}

		auto Receive(std::vector<char>& receiveBuffer) -> bool
		{
			if (m_status != WebSocketStatus::Connected)
				throw Error::Boring32Error{"WebSocket is not connected to receive data"};
			if (m_readInProgress)
				throw Error::Boring32Error{"WebSocket is already reading data"};

			receiveBuffer.clear();
			receiveBuffer.resize(m_settings.BufferBlockSize);
			auto bufferType = Win32::WinHttp::WINHTTP_WEB_SOCKET_BUFFER_TYPE{};
			auto bufferLength = static_cast<Win32::DWORD>(receiveBuffer.size() * sizeof(char));
			auto totalBytesTransferred = Win32::DWORD{};
			char* currentBufferPointer = &receiveBuffer[0];

			while (true)
			{
				auto bytesTransferred = Win32::DWORD{};
				auto statusCode = 
					Win32::WinHttp::WinHttpWebSocketReceive(
						m_winHttpWebSocket.Get(),
						currentBufferPointer,
						bufferLength,
						&bytesTransferred,
						&bufferType
					);
				// If the server terminates the connection, 12030 will returned.
				if (statusCode != Win32::ErrorCodes::Success)
					throw Error::Win32Error{statusCode, "Connection error when receiving websocket data"};
				// The server closed the connection.
				if (bufferType == Win32::WinHttp::WINHTTP_WEB_SOCKET_BUFFER_TYPE::WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE)
				{
					Close();
					return false;
				}

				currentBufferPointer += bytesTransferred;
				bufferLength -= bytesTransferred;
				totalBytesTransferred += bytesTransferred;

				// We've now got a complete buffer of either binary or UTF8 type.
				if (bufferType == Win32::WinHttp::WINHTTP_WEB_SOCKET_BUFFER_TYPE::WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE
					or bufferType == Win32::WinHttp::WINHTTP_WEB_SOCKET_BUFFER_TYPE::WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE)
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

		auto AsyncReceive() -> std::shared_ptr<ReadResult>
		{
			auto result = std::make_shared<WebSocket::ReadResult>();
			// need to assign this as this causes the process to block if it goes out of scope
			m_readFuture = std::async(
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
					std::ignore = result->Done.Signal(std::nothrow);
				});

			/*std::thread(
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
				}
			).detach();*/

			return result;
		}

		void Close()
		{
			m_status = WebSocketStatus::Closed;
			if (m_winHttpWebSocket)
			{
				Win32::WinHttp::WinHttpWebSocketClose(
					m_winHttpWebSocket.Get(),
					Win32::WinHttp::WINHTTP_WEB_SOCKET_CLOSE_STATUS::WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS,
					nullptr,
					0
				);
			}
			m_winHttpWebSocket.Close();
			m_winHttpConnection.Close();
		}

		auto GetStatus() const noexcept -> WebSocketStatus
		{
			return m_status;
		}

	private:
		void InternalConnect(const std::wstring& path)
		{
			if (m_status != WebSocketStatus::NotInitialised)
				throw Error::Boring32Error("WebSocket needs to be in NotInitialised state to connect");

			try
			{
				if (not m_settings.WinHttpSession.GetSession())
					throw Error::Boring32Error{ "WinHttp session cannot be null" };

				m_winHttpConnection = 
					Win32::WinHttp::WinHttpConnect(
						m_settings.WinHttpSession.GetSession(),
						m_settings.Server.c_str(),
						m_settings.Port,
						0
					);
				if (not m_winHttpConnection)
					throw Error::Win32Error{ Win32::GetLastError(), "WinHttpConnect() failed" };

				// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpopenrequest
				auto requestHandle = WinHttpHandle{ 
					Win32::WinHttp::WinHttpOpenRequest(
						m_winHttpConnection.Get(),
						L"GET",
						path.c_str(),
						nullptr,
						nullptr,
						nullptr,
						Win32::WinHttp::FlagSecure
					) };
				if (not requestHandle)
					throw Error::Win32Error{ Win32::GetLastError(), "WinHttpOpenRequest() failed" };

				auto success = false;
				if (m_settings.IgnoreSslErrors)
				{
					auto optionFlags = Win32::DWORD{Win32::WinHttp::SecurityFlagIgnoreAllCertErrors};
					// https://docs.microsoft.com/en-us/windows/win32/winhttp/option-flags
					// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpsetoption
					success = Win32::WinHttp::WinHttpSetOption(
						requestHandle.Get(),
						Win32::WinHttp::Options::SecurityFlags,
						&optionFlags,
						sizeof(optionFlags)
					);
					if (not success)
						throw Error::Win32Error{ Win32::GetLastError(), "WinHttpSetOption() failed" };
				}

				success = Win32::WinHttp::WinHttpSetOption(
					requestHandle.Get(),
					Win32::WinHttp::Options::UpgradeToWebSocket,
					nullptr,
					0
				);
				if (not success)
					throw Error::Win32Error{ Win32::GetLastError(), "WinHttpSetOption() failed" };

				if (m_settings.ClientCert.GetCert())
				{
					// If so, we need to set the certificate option, and retry the request.
					auto setCertOption = Win32::WinHttp::WinHttpSetOption(
						requestHandle.Get(),
						Win32::WinHttp::Options::ClientCertContext,
						(void*)m_settings.ClientCert.GetCert(),
						sizeof(Win32::CERT_CONTEXT)
					);
					if (not setCertOption)
						throw Error::Win32Error{ Win32::GetLastError(), "WinHttpSetOption() failed for client certificate" };
				}
				auto connectionHeaders = m_settings.ConnectionHeaders.empty()
					? reinterpret_cast<const wchar_t*>(Win32::WinHttp::NoAdditionalHeaders)
					: m_settings.ConnectionHeaders.c_str();
				success = Win32::WinHttp::WinHttpSendRequest(
					requestHandle.Get(),
					connectionHeaders,
					-1L,
					nullptr,
					0,
					0,
					0
				);
				if (not success)
					throw Error::Win32Error{ Win32::GetLastError(), "WinHttpSendRequest() failed on initial request" };

				success = Win32::WinHttp::WinHttpReceiveResponse(requestHandle.Get(), 0);
				if (not success)
					throw Error::Win32Error{ Win32::GetLastError(), "WinHttpReceiveResponse() failed on initial connection" };

				auto statusCode = Win32::DWORD{0};
				auto statusCodeSize = Win32::DWORD{sizeof(statusCode)};
				// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpqueryheaders
				success = Win32::WinHttp::WinHttpQueryHeaders(
					requestHandle.Get(),
					Win32::WinHttp::QueryStatusCode | Win32::WinHttp::QueryFlagNumber,
					reinterpret_cast<LPCWSTR>(Win32::WinHttp::HeaderNameByIndex),
					&statusCode,
					&statusCodeSize,
					reinterpret_cast<Win32::LPDWORD>(Win32::WinHttp::NoHeaderIndex)
				);
				if (not success)
					throw Error::Win32Error{ Win32::GetLastError(), "WinHttpQueryHeaders() failed" };

				if (statusCode != 101) // switching protocol
				{
					throw Error::Boring32Error{
						std::format("Received unexpected HTTP response code while upgrading to websocket: {}", statusCode)
					};
				}

				// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpwebsocketcompleteupgrade
				m_winHttpWebSocket = Win32::WinHttp::WinHttpWebSocketCompleteUpgrade(requestHandle.Get(), 0);
				if (not m_winHttpWebSocket)
					throw Error::Win32Error{ Win32::GetLastError(), "WinHttpWebSocketCompleteUpgrade() failed" };

				requestHandle = nullptr;
				m_status = WebSocketStatus::Connected;
			}
			catch (...)
			{
				m_status = WebSocketStatus::Error;
				throw;
			}
		}

		WinHttpHandle m_winHttpConnection;
		WinHttpHandle m_winHttpWebSocket;
		WebSocketStatus m_status = WebSocketStatus::NotInitialised;
		WebSocketSettings m_settings{};
		std::future<void> m_readFuture;
		bool m_readInProgress = false;
	};
}
