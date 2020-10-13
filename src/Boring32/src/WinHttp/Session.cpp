#include "pch.hpp"
#include "include/WinHttp/Session.hpp"
#include "include/Error/Win32Error.hpp"

namespace Boring32::WinHttp
{
	Session::~Session()
	{
		Close();
	}

	Session::Session(const std::wstring& userAgent)
	{
		InternalCreate(userAgent, WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, L"", L"");
	}

	Session::Session(const std::wstring& userAgent, const ProxyType proxyType)
	{
		if (proxyType == ProxyType::NamedProxy)
			throw std::invalid_argument("Named proxy cannot be specified as an option for this constructor");
		InternalCreate(userAgent, (DWORD)proxyType, L"", L"");
	}

	Session::Session(const std::wstring& userAgent, const std::wstring& namedProxy)
	{
		InternalCreate(userAgent, WINHTTP_ACCESS_TYPE_NAMED_PROXY, namedProxy, L"");
	}

	Session::Session(const std::wstring& userAgent, const std::wstring& namedProxy, const std::wstring& proxyBypass)
	{
		InternalCreate(userAgent, WINHTTP_ACCESS_TYPE_NAMED_PROXY, namedProxy, proxyBypass);
	}

	HINTERNET Session::GetSession() const
	{
		return m_session.Get();
	}

	void Session::Close()
	{
		m_session.Close();
	}

	void Session::InternalCreate(
		const std::wstring& ua,
		const DWORD accessType,
		const std::wstring& proxyName,
		const std::wstring& proxyBypass
	)
	{
		if (ua.empty())
			throw std::invalid_argument("userAgent cannot be empty");
		if (accessType == WINHTTP_ACCESS_TYPE_NAMED_PROXY && proxyName.empty())
			throw std::invalid_argument("proxyName parameter is required when access type is named proxy");

		m_session = WinHttpOpen(
			ua.c_str(),
			accessType,
			accessType == WINHTTP_ACCESS_TYPE_NAMED_PROXY ? proxyName.c_str() : WINHTTP_NO_PROXY_NAME,
			proxyBypass.empty() ? WINHTTP_NO_PROXY_BYPASS : proxyBypass.c_str(),
			0
		);
		if (m_session == nullptr)
			throw Error::Win32Error("Session::Session(): WinHttpOpen() failed", GetLastError());
	}
}
