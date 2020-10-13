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
			Session(const std::wstring& userAgent);
			Session(const std::wstring& userAgent, const ProxyType proxyType);
			Session(const std::wstring& userAgent, const std::wstring& namedProxy);
			Session(const std::wstring& userAgent, const std::wstring& namedProxy, const std::wstring& proxyBypass);

			// Figure out if these sematics make sense later
			Session(const Session& other) = delete;
			virtual Session& operator=(const Session& other) = delete;
			Session(Session&& other) noexcept = delete;
			virtual Session& operator=(Session&& other) noexcept = delete;

		public:
			virtual HINTERNET GetSession() const;

			/// <summary>
			///		Releases this object's ownership of the underlying WinHttp session,
			///		if any. If this object owned the last pointer to the WinHttp session,
			///		the session is destroyed. This method has no effect if no WinHttp
			///		session is owned by this object.
			/// </summary>
			virtual void Close();

		protected:
			virtual void InternalCreate(
				const std::wstring& ua, 
				const DWORD accessType, 
				const std::wstring& proxyName, 
				const std::wstring& proxyBypass
			);

		protected:
			std::shared_ptr<void> m_session;
	};
}