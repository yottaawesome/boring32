#pragma once
#include <vector>
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

	struct WebSocketReadResult
	{
		WebSocketReadResultStatus Status = WebSocketReadResultStatus::NotInitiated;
		DWORD TotalBytesRead = 0;
		std::vector<char> Data;
		Async::Event Complete{ false,true,false };
	};

	class AsyncWebSocket
	{
		public:
			virtual ~AsyncWebSocket();
			AsyncWebSocket(const AsyncWebSocketSettings& settings);
		
		public:
			virtual const AsyncWebSocketSettings& GetSettings();
			virtual void Connect();
			virtual void Connect(const std::wstring& path);
			virtual void SendString(const std::string& msg);
			virtual void SendBuffer(const std::vector<char>& buffer);
			virtual WebSocketReadResult& Receive();
			virtual WebSocketReadResult& Receive(WebSocketReadResult& receiveBuffer);
			virtual void CloseSocket();
			virtual void Release();
			virtual WebSocketStatus GetStatus() const noexcept;
			virtual WebSocketReadResult GetFirstFinished();

		protected:
			virtual void InternalConnect(const std::wstring& path);
			virtual void Move(AsyncWebSocketSettings& other) noexcept;
			static void StatusCallback(
				HINTERNET hInternet,
				DWORD_PTR dwContext,
				DWORD dwInternetStatus,
				LPVOID lpvStatusInformation,
				DWORD dwStatusInformationLength
			);

		protected:
			AsyncWebSocketSettings m_settings;
			WinHttpHandle m_winHttpConnection;
			WinHttpHandle m_winHttpSession;
			WinHttpHandle m_winHttpWebSocket;
			WebSocketStatus m_status;
			WinHttpHandle m_requestHandle;
			CRITICAL_SECTION m_cs;
			static DWORD m_bufferBlockSize;
			std::vector<WebSocketReadResult> m_readResults;
	};
}
