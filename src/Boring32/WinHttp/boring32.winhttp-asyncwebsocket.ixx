module;

#include <vector>
#include <string>
#include <future>
#include <memory>
#include <Windows.h>
#include <winhttp.h>

export module boring32.winhttp:asyncwebsocket;
import :websocketstatus;
import :asyncwebsocketsettings;
import boring32.async.criticalsectionlock;
import boring32.async.event;
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
			virtual ~AsyncWebSocket();
			AsyncWebSocket(const AsyncWebSocketSettings& settings);
		
		public:
			virtual const AsyncWebSocketSettings& GetSettings();
			virtual const ConnectionResult& Connect();
			virtual const ConnectionResult& Connect(const std::wstring& path);
			virtual const ConnectionResult& GetConnectionStatus() const;
			virtual const WriteResult& SendString(const std::string& msg);
			virtual const WriteResult& SendBuffer(const std::vector<std::byte>& buffer);
			virtual const AsyncReadResult& Receive();
			virtual void CloseSocket();
			virtual void Release();
			virtual WebSocketStatus GetStatus() const noexcept;
			virtual const AsyncReadResult& GetCurrentRead();
			//virtual std::shared_future<WebSocketReadResult> Receive2();

		protected:
			virtual const ConnectionResult& InternalConnect(const std::wstring& path);
			virtual const AsyncReadResult& Receive(AsyncReadResult& receiveBuffer);
			virtual void Move(AsyncWebSocket& other) noexcept;
			virtual void CompleteUpgrade();
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
}
