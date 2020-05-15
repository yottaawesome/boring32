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
			virtual ~WinHttpHandle();
			WinHttpHandle();
			WinHttpHandle(HINTERNET handle);
			
			WinHttpHandle(WinHttpHandle&& other) noexcept;
			virtual void operator=(WinHttpHandle&& other) noexcept;

			WinHttpHandle(const WinHttpHandle&) = delete;
			virtual void operator=(const WinHttpHandle&) = delete;

			virtual void operator=(const HINTERNET& copy);
			virtual bool operator==(const HINTERNET other);
			virtual HINTERNET Get();
			virtual void Close();

		private:
			HINTERNET m_handle;
	};

	class WinHttpRequestResult
	{
		public:
			UINT StatusCode;
			std::string ResponseBody;
	};

	class WinHttpWebClient
	{
		public:
			virtual ~WinHttpWebClient();
			WinHttpWebClient();
			WinHttpWebClient(
				const std::wstring& userAgentName, 
				const std::wstring& server,
				const UINT port,
				const bool ignoreSslErrors,
				const std::vector<std::wstring>& acceptTypes,
				const std::wstring& additionalHeaders
			);
			WinHttpWebClient(const WinHttpWebClient& other);
			virtual void operator=(const WinHttpWebClient& other);

			WinHttpWebClient(WinHttpWebClient&& other) noexcept;
			virtual void operator=(WinHttpWebClient&& other) noexcept;

			virtual void Get(const std::wstring& path);
			virtual void Post(const std::wstring& path, const std::string& requestBody);
			virtual void Put(const std::wstring& path, const std::string& requestBody);
			virtual void Delete(const std::wstring& path, const std::string& requestBody);
			virtual void Close();

		protected:
			virtual void Connect();
			virtual WinHttpRequestResult ExecuteRequest(
				const std::wstring& verb,
				const std::wstring& path,
				const std::string& requestBody,
				const std::wstring& additionalHeaders
			);
			virtual void Copy(const WinHttpWebClient& other);
			virtual void Move(WinHttpWebClient& other);

		protected:
			WinHttpHandle m_hSession;
			WinHttpHandle m_hConnect;
			std::wstring m_userAgentName;
			std::wstring m_serverToConnectTo;
			UINT m_port;
			bool m_ignoreSslErrors;
			std::vector<std::wstring> m_acceptTypes;
			std::wstring m_additionalHeaders;
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
			virtual bool Receive(std::vector<char>& buffer);
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