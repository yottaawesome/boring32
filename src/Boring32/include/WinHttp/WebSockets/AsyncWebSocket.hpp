#pragma once
#include <vector>
#include <future>
#include <memory>
#include "../../Async/CriticalSectionLock.hpp"
#include "../../Async/Event.hpp"
#include "AsyncWebSocketSettings.hpp"
#include "WebSocketStatus.hpp"

namespace Boring32::WinHttp::WebSockets
{
	enum class WebSocketReadResultStatus : DWORD
	{
		NotInitiated,
		Initiated,
		PartialRead,
		Finished
	};

	struct AsyncReadResult
	{
		WebSocketReadResultStatus Status = WebSocketReadResultStatus::NotInitiated;
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
		WriteResultStatus Status = WriteResultStatus::NotInitiated;
		Async::Event Complete{ false,true,false };
	};

	struct ConnectionResult
	{
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
			virtual std::shared_ptr<const ConnectionResult> Connect();
			virtual std::shared_ptr<const ConnectionResult> Connect(const std::wstring& path);
			virtual std::shared_ptr<const ConnectionResult> GetConnectionStatus() const;
			virtual std::shared_ptr<const WriteResult> SendString(const std::string& msg);
			virtual std::shared_ptr<const WriteResult> SendBuffer(const std::vector<std::byte>& buffer);
			virtual std::shared_ptr<const AsyncReadResult> Receive();
			virtual void CloseSocket();
			virtual void Release();
			virtual WebSocketStatus GetStatus() const noexcept;
			virtual std::shared_ptr<const AsyncReadResult> GetCurrentRead();
			//virtual std::shared_future<WebSocketReadResult> Receive2();

		protected:
			virtual std::shared_ptr<const ConnectionResult> InternalConnect(const std::wstring& path);
			virtual const AsyncReadResult& Receive(AsyncReadResult& receiveBuffer);
			virtual void Move(AsyncWebSocketSettings& other) noexcept;
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
			std::atomic<WebSocketStatus> m_status;
			WinHttpHandle m_requestHandle;
			//CRITICAL_SECTION m_cs;
			static DWORD m_bufferBlockSize;
			// FIX: these should be returned as shared_ptrs to avoid tying
			// them to this object when it goes out of scope and threads
			// are still waiting for the signal
			std::shared_ptr<AsyncReadResult> m_readResult;
			std::shared_ptr<ConnectionResult> m_connectionResult;
			std::shared_ptr<WriteResult> m_writeResult;
	};
}
