#pragma once
#include <string>
#include <Windows.h>
#include <winhttp.h>

namespace Boring32::WinHttp
{
	class ProxyInfo
	{
		public:
			virtual ~ProxyInfo();
			ProxyInfo();

		public:
			virtual void GetProxyForUrl(
				const HINTERNET session,
				const std::wstring& url,
				WINHTTP_AUTOPROXY_OPTIONS& options
			);
			virtual void Close();
			virtual void SetInfo(
				const std::wstring& proxy,
				const std::wstring& proxyBypass, 
				const DWORD accessType
			);

		protected:
			WINHTTP_PROXY_INFO m_info;
			bool m_mustRelease;
			std::wstring m_proxy;
			std::wstring m_proxyBypass;
	};
}
