export module boring32.winhttp:decomposedurl;
import <string>;
import <win32.hpp>;
import boring32.error;

namespace Boring32::WinHttp
{
	export
	{
		class DecomposedURL;
	}

	class DecomposedURL final
	{
		public:
			~DecomposedURL() = default;
			DecomposedURL(const DecomposedURL&) = default;
			DecomposedURL(DecomposedURL&&) noexcept = default;
			DecomposedURL& operator=(const DecomposedURL&) = default;
			DecomposedURL& operator=(DecomposedURL&&) noexcept = default;

		public:
			DecomposedURL(std::wstring url) : m_url(std::move(url)) { Crack(); }

		public:
			const std::wstring& Url() const noexcept { return m_url; }
			const std::wstring& Scheme() const noexcept { return m_scheme; }
			const std::wstring& Host() const noexcept { return m_host; }
			const std::wstring& Path() const noexcept { return m_path; }
			const std::wstring& Username() const noexcept { return m_username; }
			const std::wstring& Password() const noexcept { return m_password; }
			const std::wstring& ExtraInfo() const noexcept { return m_extrainfo; }
			unsigned short Port() const noexcept { return m_port; }

		private:
			void Crack()
			{
				// https://learn.microsoft.com/en-us/windows/win32/api/winhttp/ns-winhttp-url_components
				URL_COMPONENTS components{
					.dwStructSize = sizeof(URL_COMPONENTS),
					.dwSchemeLength = (DWORD)-1,
					.dwHostNameLength = (DWORD)-1,
					.nPort = 0,
					.dwUserNameLength = (DWORD)-1,
					.dwPasswordLength = (DWORD)-1,
					.dwUrlPathLength = (DWORD)-1,
					.dwExtraInfoLength = (DWORD)-1
				};
				// https://learn.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpcrackurl
				if (!WinHttpCrackUrl(m_url.c_str(), 0, 0, &components))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("WinHttpCrackUrl() failed", lastError);
				}

				// there's also nScheme
				m_scheme = { components.lpszScheme, components.dwSchemeLength };
				m_host = { components.lpszHostName, components.dwHostNameLength };
				m_path = { components.lpszUrlPath, components.dwUrlPathLength };
				m_username = { components.lpszUserName, components.dwUserNameLength };
				m_password = { components.lpszPassword, components.dwPasswordLength };
				m_extrainfo = { components.lpszExtraInfo, components.dwExtraInfoLength };
				m_port = components.nPort;
			}

		private:
			std::wstring m_url;
			std::wstring m_scheme;
			std::wstring m_host;
			std::wstring m_path;
			std::wstring m_username;
			std::wstring m_password;
			std::wstring m_extrainfo;
			unsigned short m_port;
	};
}