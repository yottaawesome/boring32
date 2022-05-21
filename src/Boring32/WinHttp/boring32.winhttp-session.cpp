module;

#include <string>
#include <memory>
#include <stdexcept>
#include <source_location>
#include <Windows.h>
#include <winhttp.h>

module boring32.winhttp:session;
import boring32.error;

namespace Boring32::WinHttp
{
	Session::~Session()
	{
		Close();
	}

	Session::Session()
	:	m_userAgent(L""),
		m_proxyType(ProxyType::AutoProxy)
	{ }

	Session::Session(std::wstring userAgent)
	:	m_userAgent(std::move(userAgent)), 
		m_proxyType(ProxyType::AutoProxy)
	{
		InternalCreate();
	}

	Session::Session(std::wstring userAgent, const ProxyType proxyType)
	:	m_userAgent(std::move(userAgent)),
		m_proxyType(proxyType)
	{
		if (m_proxyType == ProxyType::NamedProxy)
			throw std::invalid_argument("Named proxy cannot be specified as an option for this constructor");
		InternalCreate();
	}

	Session::Session(std::wstring userAgent, std::wstring namedProxy)
	:	m_userAgent(std::move(userAgent)),
		m_proxyType(ProxyType::NamedProxy),
		m_namedProxy(std::move(namedProxy))
	{
		InternalCreate();
	}

	Session::Session(std::wstring userAgent, std::wstring namedProxy, std::wstring proxyBypass)
	:	m_userAgent(std::move(userAgent)),
		m_proxyType(ProxyType::NamedProxy),
		m_namedProxy(std::move(namedProxy)),
		m_proxyBypass(std::move(proxyBypass))
	{
		InternalCreate();
	}

	Session::Session(const Session& other)
	{
		Copy(other);
	}

	Session& Session::operator=(const Session& other)
	{
		Copy(other);
		return *this;
	}

	Session& Session::Copy(const Session& other)
	{
		if (this == &other)
			return *this;
		Close();
		m_proxyType = other.m_proxyType;
		m_userAgent = other.m_userAgent;
		m_namedProxy = other.m_namedProxy;
		m_proxyBypass = other.m_proxyBypass;
		m_session = other.m_session;
		return *this;
	}

	Session::Session(Session&& other) noexcept
	{
		Move(other);
	}

	Session& Session::operator=(Session&& other) noexcept
	{
		Move(other);
		return *this;
	}

	Session& Session::Move(Session& other) noexcept
	{
		Close();
		m_session = std::move(other.m_session);
		m_proxyType = std::move(other.m_proxyType);
		m_userAgent = std::move(other.m_userAgent);
		m_namedProxy = std::move(other.m_namedProxy);
		m_proxyBypass = std::move(other.m_proxyBypass);
		return *this;
	}

	HINTERNET Session::GetSession() const noexcept
	{
		return m_session.get();
	}

	void Session::Close() noexcept
	{
		m_session = nullptr;
	}

	ProxyType Session::GetProxyType() const noexcept
	{
		return m_proxyType;
	}

	const std::wstring& Session::GetUserAgent() const noexcept
	{
		return m_userAgent;
	}
	
	const std::wstring& Session::GetNamedProxy() const noexcept
	{
		return m_namedProxy;
	}

	const std::wstring& Session::GetProxyBypass() const noexcept
	{
		return m_proxyBypass;
	}

	void Session::InternalCreate()
	{
		if (m_userAgent.empty())
			throw std::invalid_argument("userAgent cannot be empty");
		if ((DWORD)m_proxyType == WINHTTP_ACCESS_TYPE_NAMED_PROXY && m_namedProxy.empty())
			throw std::invalid_argument("proxyName parameter is required when access type is named proxy");

		const wchar_t* proxyType = (DWORD)m_proxyType == WINHTTP_ACCESS_TYPE_NAMED_PROXY
			? m_namedProxy.c_str() 
			: WINHTTP_NO_PROXY_NAME;
		const wchar_t* proxyBypass = m_proxyBypass.empty()
			? WINHTTP_NO_PROXY_BYPASS
			: m_proxyBypass.c_str();
		HINTERNET handle = WinHttpOpen(
			m_userAgent.c_str(),
			(DWORD)m_proxyType,
			proxyType,
			proxyBypass,
			0
		);
		if (handle == nullptr)
			throw Error::Win32Error(std::source_location::current(), "WinHttpOpen() failed", GetLastError());
		
		m_session = std::shared_ptr<void>(handle, WinHttpCloseHandle);
	}
}
