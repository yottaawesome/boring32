#pragma once
#include <string>
#include <vector>
#include <Windows.h>
#include <winhttp.h>

namespace Win32Utils::WinHttp
{
	enum class WinHttpWebSocketStatus
	{
		NotInitialised,
		Connected,
		Closed,
		Error
	};

	class WinHttpHandle
	{
		public:
			WinHttpHandle();
			WinHttpHandle(HINTERNET handle);
			WinHttpHandle(const WinHttpHandle& copy) = delete;
			~WinHttpHandle();
			void operator=(const HINTERNET& copy);
			bool operator==(const HINTERNET other);
			HINTERNET Get();

		private:
			HINTERNET m_handle;
	};

	class WinHttpWebSocket
	{
		public:
			virtual ~WinHttpWebSocket();
			WinHttpWebSocket(std::wstring server, const UINT port, const bool ignoreSslErrors);
			virtual const std::wstring& GetServer();
			virtual void Connect();
			virtual void Connect(const std::wstring& path);
			virtual void CleanServerString();
			virtual void SendString(const std::string& msg);
			virtual void SendBuffer(const std::vector<char>& buffer);
			virtual bool Receive(std::string& buffer);
			virtual void SetServer(const std::wstring& newServer, const UINT port, const bool ignoreSslErrors);
			virtual void Close();
			virtual WinHttpWebSocketStatus GetStatus();

		protected:
			virtual void InternalConnect(const std::wstring& path);

		protected:
			WinHttpHandle m_hConnect;
			WinHttpHandle m_hSession;
			WinHttpHandle m_webSocketHandle;
			std::wstring m_server;
			UINT m_port;
			bool m_ignoreSslErrors;
			WinHttpWebSocketStatus m_status;
	};
}