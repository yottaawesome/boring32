#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <memory>
#include <future>
#include "../WinHttpHandle.hpp"
#include "../Session.hpp"
#include "../../Async/Event.hpp"
#include "WebSocketStatus.hpp"
#include "WebSocketSettings.hpp"

namespace Boring32::WinHttp::WebSockets
{
	class WebSocket
	{
		public:
			struct ReadResult
			{
				std::vector<char> Buffer;
				bool Succeeded = false;
				Async::Event Done{ false, true, false };
				std::future<void> Future;
			};

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
			virtual void SendBuffer(const std::vector<std::byte>& buffer);
			virtual bool Receive(std::vector<char>& buffer);
			virtual std::shared_ptr<ReadResult> AsyncReceive();
			virtual void Close();
			virtual WebSocketStatus GetStatus() const noexcept;

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
