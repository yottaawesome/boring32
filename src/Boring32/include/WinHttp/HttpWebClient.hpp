#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include "WinHttpHandle.hpp"
#include "HttpRequestResult.hpp"
#include "ProxyInfo.hpp"

namespace Boring32::WinHttp
{
	class HttpWebClient
	{
		public:
			virtual ~HttpWebClient();
			HttpWebClient();
			HttpWebClient(
				const std::wstring& userAgentName,
				const std::wstring& serverToConnectTo,
				const std::wstring& proxy,
				const UINT port,
				const bool ignoreSslErrors
			);
			HttpWebClient(
				const std::wstring& userAgentName, 
				const std::wstring& serverToConnectTo,
				const std::wstring& proxy,
				const UINT port,
				const bool ignoreSslErrors,
				const std::vector<std::wstring>& acceptTypes,
				const std::wstring& additionalHeaders
			);
			HttpWebClient(const HttpWebClient& other);
			virtual void operator=(const HttpWebClient& other);

			HttpWebClient(HttpWebClient&& other) noexcept;
			virtual void operator=(HttpWebClient&& other) noexcept;

			virtual HttpRequestResult Get(const std::wstring& path);
			virtual HttpRequestResult Post(const std::wstring& path, const std::string& requestBody);
			virtual HttpRequestResult Put(const std::wstring& path, const std::string& requestBody);
			virtual HttpRequestResult Delete(const std::wstring& path, const std::string& requestBody);
			virtual void Close();
			virtual void Connect();

		protected:
			virtual HttpRequestResult ExecuteRequest(
				const std::wstring& verb,
				const std::wstring& path,
				const std::string& requestBody,
				const std::wstring& additionalHeaders
			);
			virtual void Copy(const HttpWebClient& other);
			virtual void Move(HttpWebClient& other);

		protected:
			WinHttpHandle m_hSession;
			WinHttpHandle m_hConnect;
			std::wstring m_userAgentName;
			std::wstring m_serverToConnectTo;
			std::wstring m_proxy;
			UINT m_port;
			bool m_ignoreSslErrors;
			std::vector<std::wstring> m_acceptTypes;
			std::wstring m_additionalHeaders;
			ProxyInfo m_proxyInfo;
	};
}