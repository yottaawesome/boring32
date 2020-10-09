#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include "WinHttpHandle.hpp"
#include "WebSocketStatus.hpp"

namespace Boring32::WinHttp
{
	class WebSocket
	{
		public:
			virtual ~WebSocket();
			WebSocket(
				std::wstring server, 
				const UINT port, 
				const bool ignoreSslErrors,
				const std::wstring proxy,
				const std::wstring pacUrl
			);
			virtual const std::wstring& GetServer();
			virtual void Connect();
			virtual void Connect(const std::wstring& path);
			virtual void CleanServerString();
			virtual void SendString(const std::string& msg);
			virtual void SendBuffer(const std::vector<char>& buffer);
			virtual bool Receive(std::vector<char>& buffer);
			virtual void SetServer(const std::wstring& newServer, const UINT port, const bool ignoreSslErrors);
			virtual void Close();
			virtual WebSocketStatus GetStatus();

		protected:
			virtual void InternalConnect(const std::wstring& path);

		protected:
			WinHttpHandle m_winHttpConnection;
			WinHttpHandle m_winHttpSession;
			WinHttpHandle m_winHttpWebSocket;
			std::wstring m_server;
			std::wstring m_proxy;
			std::wstring m_pacUrl;
			UINT m_port;
			bool m_ignoreSslErrors;
			WebSocketStatus m_status;
	};
}
