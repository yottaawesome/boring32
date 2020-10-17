#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include "../WinHttpHandle.hpp"
#include "../Session.hpp"
#include "WebSocketStatus.hpp"
#include "WebSocketSettings.hpp"

namespace Boring32::WinHttp::WebSockets
{
	class WebSocket
	{
		public:
			virtual ~WebSocket();
			WebSocket();
			WebSocket(WebSocketSettings settings);

			WebSocket(WebSocket&& other) noexcept;
			virtual WebSocket& operator=(WebSocket&& other) noexcept;

			WebSocket(const WebSocket&) = delete;
			virtual WebSocket& operator=(const WebSocket&) = delete;

		public:
			virtual const WebSocketSettings& GetSettings();
			virtual void Connect();
			virtual void Connect(const std::wstring& path);
			virtual void SendString(const std::string& msg);
			virtual void SendBuffer(const std::vector<char>& buffer);
			virtual bool Receive(std::vector<char>& buffer);
			virtual void Close();
			virtual WebSocketStatus GetStatus();

		protected:
			virtual void InternalConnect(const std::wstring& path);
			virtual void Move(WebSocket& other) noexcept;

		protected:
			WinHttpHandle m_winHttpConnection;
			WinHttpHandle m_winHttpWebSocket;
			WebSocketStatus m_status;
			WebSocketSettings m_settings;
	};
}
