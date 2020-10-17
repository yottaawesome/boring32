#pragma once
#include <winhttp.h>
#include <memory>
#include "ProxyType.hpp"
#include "WinHttpHandle.hpp"

namespace Boring32::WinHttp
{
	class Session
	{
		public:
			virtual ~Session();
			Session(std::wstring userAgent);
			Session(std::wstring userAgent, const ProxyType proxyType);
			Session(std::wstring userAgent, std::wstring namedProxy);
			Session(std::wstring userAgent, std::wstring namedProxy, std::wstring proxyBypass);

			Session(const Session& other);
			virtual Session& operator=(const Session& other);
			Session(Session&& other) noexcept;
			virtual Session& operator=(Session&& other) noexcept;

		public:
			virtual HINTERNET GetSession() const;

			/// <summary>
			///		Releases this object's ownership of the underlying WinHttp session,
			///		if any. If this object owned the last pointer to the WinHttp session,
			///		the session is destroyed. This method has no effect if no WinHttp
			///		session is owned by this object.
			/// </summary>
			virtual void Close();

			virtual ProxyType GetProxyType();
			virtual const std::wstring& GetUserAgent();
			virtual const std::wstring& GetNamedProxy();
			virtual const std::wstring& GetProxyBypass();

		protected:
			virtual void InternalCreate();
			virtual Session& Copy(const Session& session);
			virtual Session& Move(Session& session) noexcept;

		protected:
			std::shared_ptr<void> m_session;
			ProxyType m_proxyType;
			std::wstring m_userAgent;
			std::wstring m_namedProxy; 
			std::wstring m_proxyBypass;
	};
}