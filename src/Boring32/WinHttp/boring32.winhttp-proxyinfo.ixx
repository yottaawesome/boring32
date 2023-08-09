export module boring32.winhttp:proxyinfo;
import std;

import <win32.hpp>;
import boring32.error;

export namespace Boring32::WinHttp
{
	class ProxyInfo
	{
		public:
			virtual ~ProxyInfo()
			{
				Close();
			}

			// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/ns-winhttp-winhttp_proxy_info
			ProxyInfo() = default;

		public:
			virtual void GetProxyForUrl(
				const HINTERNET session,
				const std::wstring& url,
				WINHTTP_AUTOPROXY_OPTIONS& options
			)
			{
				// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpgetproxyforurl
				if (!WinHttpGetProxyForUrl(session, url.c_str(), &options, &m_info))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error(
						"WinHttpGetProxyForUrl() failed",
						lastError
					);
				}
				m_mustRelease = true;
			}
			
			virtual void Close()
			{
				if (!m_mustRelease)
					return;

				if (m_info.lpszProxy)
					GlobalFree(m_info.lpszProxy);
				if (m_info.lpszProxyBypass)
					GlobalFree(m_info.lpszProxyBypass);
				m_info.lpszProxy = nullptr;
				m_info.lpszProxyBypass = nullptr;
				m_mustRelease = false;
			}
			
			virtual void SetNamedProxy(
				const std::wstring& proxy,
				const std::wstring& proxyBypass
			)
			{
				Close();
				m_proxy = proxy;
				m_proxyBypass = proxyBypass;
				m_info.lpszProxy = m_proxy.empty() ? nullptr : &m_proxy[0];
				m_info.lpszProxyBypass = m_proxyBypass.empty() ? nullptr : &m_proxyBypass[0];
				m_info.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
			}

			virtual void SetAutoProxy(
				HINTERNET session, 
				const std::wstring& pacUrl, 
				const std::wstring& url
			)
			{
				Close();
				// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/ns-winhttp-winhttp_autoproxy_options
				WINHTTP_AUTOPROXY_OPTIONS autoProxyOptions{ 0 };
				autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;
				autoProxyOptions.lpszAutoConfigUrl = pacUrl.c_str();
				autoProxyOptions.dwAutoDetectFlags = 0;
				autoProxyOptions.fAutoLogonIfChallenged = false;
				autoProxyOptions.lpvReserved = 0;
				autoProxyOptions.dwReserved = 0;
				// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpgetproxyforurl
				if (!WinHttpGetProxyForUrl(session, url.c_str(), &autoProxyOptions, &m_info))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error(
						"WinHttpGetProxyForUrl() failed",
						lastError
					);
				}
			}

			virtual void SetAllInfo(
				const std::wstring& proxy,
				const std::wstring& proxyBypass,
				const DWORD accessType
			)
			{
				Close();
				m_proxy = proxy;
				m_proxyBypass = proxyBypass;
				m_info.lpszProxy = m_proxy.empty() ? nullptr : &m_proxy[0];
				m_info.lpszProxyBypass = m_proxyBypass.empty() ? nullptr : &m_proxyBypass[0];
				m_info.dwAccessType = accessType;
			}

			virtual void SetOnSession(HINTERNET session)
			{
				if (!m_info.lpszProxy)
					throw Boring32::Error::Boring32Error("No proxy set");
				// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpsetoption
				if (!WinHttpSetOption(session, WINHTTP_OPTION_PROXY, &m_info, sizeof(m_info)))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error(
						"WinHttpSetOption() failed",
						lastError
					);
				}
			}

		protected:
			WINHTTP_PROXY_INFO m_info{ 0 };
			bool m_mustRelease = false;
			std::wstring m_proxy;
			std::wstring m_proxyBypass;
	};
}
