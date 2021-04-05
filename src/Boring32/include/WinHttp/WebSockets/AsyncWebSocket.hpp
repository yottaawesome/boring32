#pragma once
#include "AsyncWebSocketSettings.hpp"
#include "WebSocketStatus.hpp"

namespace Boring32::WinHttp::WebSockets
{
	class WebSocketReadResult
	{
		std::string Data;	
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
			virtual bool Receive(std::vector<char>& buffer);
			virtual void CloseSocket();
			virtual void Release();
			virtual WebSocketStatus GetStatus() const noexcept;

		protected:
			virtual void InternalConnect(const std::wstring& path);
			virtual void Move(AsyncWebSocketSettings& other) noexcept;

		protected:
			AsyncWebSocketSettings m_settings;
			WinHttpHandle m_winHttpConnection;
			WinHttpHandle m_winHttpSession;
			WinHttpHandle m_winHttpWebSocket;
			WebSocketStatus m_status;
			WinHttpHandle m_requestHandle;

			static void StatusCallback(
				HINTERNET hInternet,
				DWORD_PTR dwContext,
				DWORD dwInternetStatus,
				LPVOID lpvStatusInformation,
				DWORD dwStatusInformationLength
			);
	};
}
