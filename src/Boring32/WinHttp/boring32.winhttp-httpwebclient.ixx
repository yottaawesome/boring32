export module boring32.winhttp:httpwebclient;
import <string>;
import <vector>;
import <win32.hpp>;
import <functional>;
import <sstream>;
import :winhttphandle;
import :requestresult;
import :proxyinfo;
import boring32.error;

export namespace Boring32::WinHttp
{
	class HttpWebClient
	{
		public:
			virtual ~HttpWebClient()
			{
				Close();
			}

			HttpWebClient() = default;
			
			HttpWebClient(const HttpWebClient& other)
			{
				Copy(other);
			}

			HttpWebClient(HttpWebClient&& other) noexcept
			{
				Move(other);
			}

			HttpWebClient(
				const std::wstring& userAgentName,
				const std::wstring& serverToConnectTo,
				const std::wstring& proxy,
				const UINT port,
				const bool ignoreSslErrors
			) : m_userAgentName(userAgentName),
				m_serverToConnectTo(serverToConnectTo),
				m_port(port),
				m_ignoreSslErrors(ignoreSslErrors),
				m_proxy(proxy)
			{ }

			HttpWebClient(
				const std::wstring& userAgentName, 
				const std::wstring& serverToConnectTo,
				const std::wstring& proxy,
				const UINT port,
				const bool ignoreSslErrors,
				const std::vector<std::wstring>& acceptTypes,
				const std::wstring& additionalHeaders
			) : m_userAgentName(userAgentName),
				m_serverToConnectTo(serverToConnectTo),
				m_port(port),
				m_ignoreSslErrors(ignoreSslErrors),
				m_acceptTypes(acceptTypes),
				m_additionalHeaders(additionalHeaders),
				m_proxy(proxy)
			{ }

		public:
			virtual HttpWebClient& operator=(const HttpWebClient& other)
			{
				Close();
				Copy(other);
				return *this;
			}

			virtual HttpWebClient& operator=(HttpWebClient&& other) noexcept
			{
				Close();
				Move(other);
				Connect();
				return *this;
			}

		public:
			virtual HttpRequestResult Get(const std::wstring& path)
			{
				return ExecuteRequest(L"GET", path, "", m_additionalHeaders);
			}

			virtual HttpRequestResult Post(
				const std::wstring& path, 
				const std::string& requestBody
			)
			{
				return ExecuteRequest(L"POST", path, requestBody, m_additionalHeaders);
			}

			virtual HttpRequestResult Put(
				const std::wstring& path, 
				const std::string& requestBody
			)
			{
				return ExecuteRequest(L"PUT", path, requestBody, m_additionalHeaders);
			}

			virtual HttpRequestResult Delete(
				const std::wstring& path, 
				const std::string& requestBody
			)
			{
				return ExecuteRequest(L"DELETE", path, requestBody, m_additionalHeaders);
			}

			virtual void Close()
			{
				if (m_hSession != nullptr)
				{
					m_hSession.Close();
					m_hSession = nullptr;
				}
				if (m_hConnect != nullptr)
				{
					m_hConnect.Close();
					m_hConnect = nullptr;
				}
			}

			virtual void Connect()
			{
				const bool useAutomaticProxy = m_proxy.empty();

				m_hSession = WinHttpOpen(
					m_userAgentName.c_str(),
					useAutomaticProxy ? WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY : WINHTTP_ACCESS_TYPE_NO_PROXY,
					useAutomaticProxy ? WINHTTP_NO_PROXY_NAME : m_proxy.c_str(),
					WINHTTP_NO_PROXY_BYPASS,
					0
				);
				if (!m_hSession)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("WinHttpOpen() failed", lastError);
				}

				//if (m_proxy.empty() == false)
				//{
				//	m_proxyInfo.SetNamedProxy(m_proxy, L"");
				//	m_proxyInfo.SetOnSession(m_hSession.Get());
				//}

				m_hConnect = WinHttpConnect(
					m_hSession.Get(),
					m_serverToConnectTo.c_str(),
					m_port,
					0
				);
				if (!m_hConnect)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("WinHttpConnect() failed", lastError);
				}
			}

		protected:
			virtual HttpRequestResult ExecuteRequest(
				const std::wstring& verb,
				const std::wstring& path,
				const std::string& requestBody,
				const std::wstring& additionalHeaders
			)
			{
				// acceptHeader must be a null terminated array
				std::vector<LPCWSTR> acceptHeader;
				if (m_acceptTypes.size() > 0)
				{
					acceptHeader.resize(m_acceptTypes.size() + 1);
					for (int i = 0; i < m_acceptTypes.size(); i++)
						acceptHeader[i] = m_acceptTypes.at(i).c_str();
				}

				// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpopenrequest
				WinHttpHandle hRequest = WinHttpOpenRequest(
					m_hConnect.Get(),
					verb.c_str(),
					path.c_str(),
					nullptr,
					WINHTTP_NO_REFERER,
					m_acceptTypes.empty() ? WINHTTP_DEFAULT_ACCEPT_TYPES : &acceptHeader[0],
					WINHTTP_FLAG_SECURE
				);
				if (!hRequest)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("WinHttpOpenRequest() failed", lastError);
				}

				if (m_ignoreSslErrors)
				{
					DWORD flags =
						SECURITY_FLAG_IGNORE_UNKNOWN_CA |
						SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE |
						SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
						SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
					const bool succeeded = WinHttpSetOption(
						hRequest.Get(),
						WINHTTP_OPTION_SECURITY_FLAGS,
						&flags,
						sizeof(flags)
					);
					if (!succeeded)
					{
						const auto lastError = GetLastError();
						throw Error::Win32Error("WinHttpSetOption() failed", lastError);
					}
				}

				// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpsendrequest
				bool succeeded = WinHttpSendRequest(
					hRequest.Get(),
					additionalHeaders.size() > 0
					? additionalHeaders.c_str()
					: WINHTTP_NO_ADDITIONAL_HEADERS,
					-1L,
					requestBody.size() > 0
					? const_cast<char*>(&requestBody[0])
					: WINHTTP_NO_REQUEST_DATA,
					static_cast<DWORD>(requestBody.size()),
					static_cast<DWORD>(requestBody.size()),
					reinterpret_cast<DWORD_PTR>(this)
				);
				if (!succeeded)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("WinHttpSendRequest() failed", lastError);
				}

				// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpreceiveresponse
				succeeded = WinHttpReceiveResponse(hRequest.Get(), nullptr);
				if (!succeeded)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("WinHttpReceiveResponse() failed", lastError);
				}

				// Get the status code of the response
				DWORD statusCode = 0;
				DWORD statusCodeSize = sizeof(statusCode);
				// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpqueryheaders
				WinHttpQueryHeaders(
					hRequest.Get(),
					WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
					WINHTTP_HEADER_NAME_BY_INDEX,
					&statusCode,
					&statusCodeSize,
					WINHTTP_NO_HEADER_INDEX);

				DWORD bytesOfDataAvailable = 0;
				std::string response = "";
				do
				{
					// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpquerydataavailable
					if (!WinHttpQueryDataAvailable(hRequest.Get(), &bytesOfDataAvailable))
					{
						const auto lastError = GetLastError();
						throw Error::Win32Error("WinHttpQueryDataAvailable() failed", lastError);
					}
					if (bytesOfDataAvailable == 0)
						continue;

					// Allocate space for the buffer.
					std::vector<char> outBuffer(bytesOfDataAvailable);
					DWORD downloadedBytes = 0;
					// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpreaddata
					// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpreaddataex
					const bool succeeded = WinHttpReadData(
						hRequest.Get(),
						&outBuffer[0],
						bytesOfDataAvailable,
						&downloadedBytes
					);
					if (!succeeded)
					{
						const auto lastError = GetLastError();
						throw Error::Win32Error("WinHttpQueryDataAvailable() failed", lastError);
					}

					response.append(outBuffer.begin(), outBuffer.end());
				} while (bytesOfDataAvailable > 0);

				return HttpRequestResult{ statusCode, response };
			}

			virtual void Copy(const HttpWebClient& other)
			{
				m_userAgentName = other.m_userAgentName;
				m_serverToConnectTo = other.m_serverToConnectTo;
				m_port = other.m_port;
				m_ignoreSslErrors = other.m_ignoreSslErrors;
				m_acceptTypes = other.m_acceptTypes;
				m_additionalHeaders = other.m_additionalHeaders;
			}

			virtual void Move(HttpWebClient& other)
			{
				m_userAgentName = std::move(other.m_userAgentName);
				m_serverToConnectTo = std::move(other.m_serverToConnectTo);
				m_port = other.m_port;
				m_ignoreSslErrors = other.m_ignoreSslErrors;
				m_acceptTypes = std::move(other.m_acceptTypes);
				m_additionalHeaders = std::move(other.m_additionalHeaders);
			}

		protected:
			WinHttpHandle m_hSession;
			WinHttpHandle m_hConnect;
			std::wstring m_userAgentName;
			std::wstring m_serverToConnectTo;
			std::wstring m_proxy;
			UINT m_port = 0;
			bool m_ignoreSslErrors = false;
			std::vector<std::wstring> m_acceptTypes;
			std::wstring m_additionalHeaders;
			ProxyInfo m_proxyInfo;
	};
}