export module boring32:winhttp_session;
import boring32.shared;
import boring32.error;
import :winhttp_winhttphandle;
import :winhttp_proxytype;
import :winhttp_handles;
import :winhttp_winhttperror;

export namespace Boring32::WinHttp
{
	class Session final
	{
		public:
			~Session()
			{
				Close();
			}

			Session() = default;

			Session(const Session& other)
			{
				Copy(other);
			}

			Session(Session&& other) noexcept
			{
				Move(other);
			}

			Session(std::wstring userAgent)
				: m_userAgent(std::move(userAgent)),
				m_proxyType(ProxyType::AutoProxy)
			{
				InternalCreate();
			}

			Session(std::wstring userAgent, const ProxyType proxyType)
				: m_userAgent(std::move(userAgent)),
				m_proxyType(proxyType)
			{
				if (m_proxyType == ProxyType::NamedProxy)
					throw std::invalid_argument("Named proxy cannot be specified as an option for this constructor");
				InternalCreate();
			}

			Session(std::wstring userAgent, std::wstring namedProxy)
				: m_userAgent(std::move(userAgent)),
				m_proxyType(ProxyType::NamedProxy),
				m_namedProxy(std::move(namedProxy))
			{
				InternalCreate();
			}

			Session(std::wstring userAgent, std::wstring namedProxy, std::wstring proxyBypass)
				: m_userAgent(std::move(userAgent)),
				m_proxyType(ProxyType::NamedProxy),
				m_namedProxy(std::move(namedProxy)),
				m_proxyBypass(std::move(proxyBypass))
			{
				InternalCreate();
			}

		public:
			Session& operator=(const Session& other)
			{
				Copy(other);
				return *this;
			}

			Session& operator=(Session&& other) noexcept
			{
				Move(other);
				return *this;
			}

		public:
			Win32::WinHttp::HINTERNET GetSession() const noexcept
			{
				return m_session.get();
			}

			/// <summary>
			///		Releases this object's ownership of the underlying WinHttp session,
			///		if any. If this object owned the last pointer to the WinHttp session,
			///		the session is destroyed. This method has no effect if no WinHttp
			///		session is owned by this object.
			/// </summary>
			void Close() noexcept
			{
				m_session = nullptr;
				m_proxyType = ProxyType::NoProxy;
				m_userAgent.clear();
				m_namedProxy.clear();
				m_proxyBypass.clear();
			}

			ProxyType GetProxyType() const noexcept
			{
				return m_proxyType;
			}

			const std::wstring& GetUserAgent() const noexcept
			{
				return m_userAgent;
			}

			const std::wstring& GetNamedProxy() const noexcept
			{
				return m_namedProxy;
			}

			const std::wstring& GetProxyBypass() const noexcept
			{
				return m_proxyBypass;
			}

		protected:
			void InternalCreate()
			{
				if (m_userAgent.empty())
					throw WinHttpError("UserAgent cannot be empty");
				if (static_cast<Win32::DWORD>(m_proxyType) == Win32::WinHttp::_WINHTTP_ACCESS_TYPE_NAMED_PROXY && m_namedProxy.empty())
					throw WinHttpError("ProxyName parameter is required when access type is NamedProxy");

				const wchar_t* proxyType = static_cast<Win32::DWORD>(m_proxyType) == Win32::WinHttp::_WINHTTP_ACCESS_TYPE_NAMED_PROXY
					? m_namedProxy.c_str()
					: (wchar_t*)Win32::WinHttp::_WINHTTP_NO_PROXY_NAME;
				const wchar_t* proxyBypass = m_proxyBypass.empty()
					? (wchar_t*)Win32::WinHttp::_WINHTTP_NO_PROXY_BYPASS
					: m_proxyBypass.c_str();
				// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpopen
				const Win32::WinHttp::HINTERNET handle = Win32::WinHttp::WinHttpOpen(
					m_userAgent.c_str(),
					static_cast<Win32::DWORD>(m_proxyType),
					proxyType,
					proxyBypass,
					0
				);
				if (!handle)
				{
					const auto lastError = Win32::GetLastError();
					Error::ThrowNested(
						Error::Win32Error("WinHttpOpen() failed", lastError),
						WinHttpError("Failed to open WinHttpSession handle")
					);
				}

				m_session = SharedWinHttpSession(handle, Win32::WinHttp::WinHttpCloseHandle);
			}

			Session& Copy(const Session& other)
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

			Session& Move(Session& other) noexcept
			{
				Close();
				m_session = std::move(other.m_session);
				m_proxyType = other.m_proxyType;
				m_userAgent = std::move(other.m_userAgent);
				m_namedProxy = std::move(other.m_namedProxy);
				m_proxyBypass = std::move(other.m_proxyBypass);
				other.Close();
				return *this;
			}

		protected:
			SharedWinHttpSession m_session;
			ProxyType m_proxyType = ProxyType::AutoProxy;
			std::wstring m_userAgent;
			std::wstring m_namedProxy; 
			std::wstring m_proxyBypass;
	};
}