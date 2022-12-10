export module boring32.winhttp:httpwebclient;
import :winhttphandle;
import :requestresult;
import :proxyinfo;
import <string>;
import <vector>;
import <win32.hpp>;

export namespace Boring32::WinHttp
{
	class HttpWebClient
	{
		public:
			virtual ~HttpWebClient();
			HttpWebClient();
			HttpWebClient(const HttpWebClient& other);
			HttpWebClient(HttpWebClient&& other) noexcept;
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

		public:
			virtual void operator=(const HttpWebClient& other);
			virtual void operator=(HttpWebClient&& other) noexcept;

		public:
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