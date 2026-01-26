export module boring32:winhttp.proxyinfo;
import std;
import :win32;
import :error;

export namespace Boring32::WinHttp
{
	struct ProxyInfo final
	{
		~ProxyInfo()
		{
			Close();
		}

		// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/ns-winhttp-winhttp_proxy_info
		ProxyInfo() = default;

		void GetProxyForUrl(
			const Win32::WinHttp::HINTERNET session,
			const std::wstring& url,
			Win32::WinHttp::WINHTTP_AUTOPROXY_OPTIONS& options
		)
		{
			// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpgetproxyforurl
			if (not Win32::WinHttp::WinHttpGetProxyForUrl(session, url.c_str(), &options, &m_info))
				throw Error::Win32Error(Win32::GetLastError(), "WinHttpGetProxyForUrl() failed");
			m_mustRelease = true;
		}
			
		void Close()
		{
			if (not m_mustRelease)
				return;

			if (m_info.lpszProxy)
				Win32::GlobalFree(m_info.lpszProxy);
			if (m_info.lpszProxyBypass)
				Win32::GlobalFree(m_info.lpszProxyBypass);
			m_info.lpszProxy = nullptr;
			m_info.lpszProxyBypass = nullptr;
			m_mustRelease = false;
		}
			
		void SetNamedProxy(const std::wstring& proxy, const std::wstring& proxyBypass)
		{
			Close();
			m_proxy = proxy;
			m_proxyBypass = proxyBypass;
			m_info.lpszProxy = m_proxy.empty() ? nullptr : &m_proxy[0];
			m_info.lpszProxyBypass = m_proxyBypass.empty() ? nullptr : &m_proxyBypass[0];
			m_info.dwAccessType = Win32::WinHttp::AccessTypeNamedProxy;
		}

		void SetAutoProxy(Win32::WinHttp::HINTERNET session, const std::wstring& pacUrl, const std::wstring& url)
		{
			Close();
			// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/ns-winhttp-winhttp_autoproxy_options
			Win32::WinHttp::WINHTTP_AUTOPROXY_OPTIONS autoProxyOptions{ 0 };
			autoProxyOptions.dwFlags = Win32::WinHttp::AutoProxyConfigUrl;
			autoProxyOptions.lpszAutoConfigUrl = pacUrl.c_str();
			autoProxyOptions.dwAutoDetectFlags = 0;
			autoProxyOptions.fAutoLogonIfChallenged = false;
			autoProxyOptions.lpvReserved = 0;
			autoProxyOptions.dwReserved = 0;
			// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpgetproxyforurl
			if (not Win32::WinHttp::WinHttpGetProxyForUrl(session, url.c_str(), &autoProxyOptions, &m_info))
				throw Error::Win32Error(Win32::GetLastError(), "WinHttpGetProxyForUrl() failed");
		}

		void SetAllInfo(const std::wstring& proxy, const std::wstring& proxyBypass, const Win32::DWORD accessType)
		{
			Close();
			m_proxy = proxy;
			m_proxyBypass = proxyBypass;
			m_info.lpszProxy = m_proxy.empty() ? nullptr : &m_proxy[0];
			m_info.lpszProxyBypass = m_proxyBypass.empty() ? nullptr : &m_proxyBypass[0];
			m_info.dwAccessType = accessType;
		}

		void SetOnSession(Win32::WinHttp::HINTERNET session)
		{
			if (not m_info.lpszProxy)
				throw Boring32::Error::Boring32Error("No proxy set");
			// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpsetoption
			if (not Win32::WinHttp::WinHttpSetOption(session, Win32::WinHttp::Options::Proxy, &m_info, sizeof(m_info)))
				throw Error::Win32Error(Win32::GetLastError(), "WinHttpSetOption() failed");
		}

		private:
		Win32::WinHttp::WINHTTP_PROXY_INFO m_info{ 0 };
		bool m_mustRelease = false;
		std::wstring m_proxy;
		std::wstring m_proxyBypass;
	};
}
