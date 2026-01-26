export module boring32:winhttp.httpwebclient;
import std;
import boring32.win32;
import :error;
import :strings;
import :concepts;
import :winhttp.winhttphandle;
import :winhttp.requestresult;
import :winhttp.proxyinfo;

export namespace Boring32::WinHttp::Verbs
{
	template<Strings::FixedString VVerb>
	struct HttpVerb
	{
		constexpr std::wstring_view View() const noexcept
		{
			return VVerb.ToView();
		}
	};

	constexpr HttpVerb<L"GET"> Get;
	constexpr HttpVerb<L"POST"> Post;
	constexpr HttpVerb<L"PUT"> Put;
	constexpr HttpVerb<L"DELETE"> Delete;

	template<typename T>
	struct VerbTypeT : std::false_type {};

	template<Strings::FixedString VVerb>
	struct VerbTypeT<HttpVerb<VVerb>> : std::true_type {};

	template<typename T>
	constexpr bool IsVerbType = VerbTypeT<T>::value;

	template<typename T>
	concept VerbType = IsVerbType<std::remove_cvref_t<T>>;
}

export namespace Boring32::WinHttp
{
	struct RequestHeader
	{
		constexpr RequestHeader(Concepts::ConvertiblePair auto header)
			: HeaderValuePair{ header }
		{ }

		constexpr RequestHeader(std::convertible_to<std::wstring> auto header, std::convertible_to<std::wstring> auto value)
			: HeaderValuePair{ std::wstring{header}, std::wstring{value} }
		{ }
		std::pair<std::wstring, std::wstring> HeaderValuePair;

		std::wstring ToString() const noexcept 
		{ 
			return std::format(L"{}: {}", HeaderValuePair.first, HeaderValuePair.second); 
		}
	};

	struct RequestHeaders
	{
		constexpr RequestHeaders(Concepts::ConstructibleTo<RequestHeader> auto...headers)
		{
			([](std::wstring& compiled, auto&& head)
			{
				compiled += compiled.empty() 
					? head.ToString()
					: std::format(L"\r\n{}", head.ToString());
			}(Headers, RequestHeader{headers}), ...);
		}

		std::wstring Headers;
	};

	struct HttpWebClient final
	{
		~HttpWebClient()
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
			const unsigned port,
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
			const unsigned port,
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

		HttpWebClient& operator=(const HttpWebClient& other)
		{
			Close();
			Copy(other);
			return *this;
		}

		HttpWebClient& operator=(HttpWebClient&& other) noexcept
		{
			Close();
			Move(other);
			Connect();
			return *this;
		}

		HttpRequestResult Get(const std::wstring& path)
		{
			return ExecuteRequest(Verbs::Get, path, "", m_additionalHeaders);
		}

		HttpRequestResult Post(const std::wstring& path, const std::string& requestBody)
		{
			return ExecuteRequest(Verbs::Post, path, requestBody, m_additionalHeaders);
		}

		HttpRequestResult Put(const std::wstring& path, const std::string& requestBody)
		{
			return ExecuteRequest(Verbs::Put, path, requestBody, m_additionalHeaders);
		}

		HttpRequestResult Delete(const std::wstring& path, const std::string& requestBody)
		{
			return ExecuteRequest(Verbs::Delete, path, requestBody, m_additionalHeaders);
		}

		void Close()
		{
			if (m_hSession)
			{
				m_hSession.Close();
				m_hSession = nullptr;
			}
			if (m_hConnect)
			{
				m_hConnect.Close();
				m_hConnect = nullptr;
			}
		}

		void Connect()
		{
			bool useAutomaticProxy = m_proxy.empty();

			m_hSession = Win32::WinHttp::WinHttpOpen(
				m_userAgentName.c_str(),
				useAutomaticProxy ? Win32::WinHttp::AccessTypeAutomaticProxy : Win32::WinHttp::AccessTypeNoProxy,
				useAutomaticProxy ? (Win32::LPCWSTR)Win32::WinHttp::NoProxyName : m_proxy.c_str(),
				(Win32::LPCWSTR)Win32::WinHttp::NoProxyBypass,
				0
			);
			if (not m_hSession)
				throw Error::Win32Error(Win32::GetLastError(), "WinHttpOpen() failed");

			//if (m_proxy.empty() == false)
			//{
			//	m_proxyInfo.SetNamedProxy(m_proxy, L"");
			//	m_proxyInfo.SetOnSession(m_hSession.Get());
			//}

			m_hConnect = Win32::WinHttp::WinHttpConnect(
				m_hSession.Get(),
				m_serverToConnectTo.c_str(),
				m_port,
				0
			);
			if (not m_hConnect)
				throw Error::Win32Error(Win32::GetLastError(), "WinHttpConnect() failed");
		}

	private:
		HttpRequestResult ExecuteRequest(
			Verbs::VerbType auto verb,
			const std::wstring& path,
			const std::string& requestBody,
			const std::wstring& additionalHeaders
		)
		{
			// acceptHeader must be a null terminated array
			std::vector<Win32::LPCWSTR> acceptHeader;
			if (m_acceptTypes.size() > 0)
			{
				acceptHeader.resize(m_acceptTypes.size() + 1);
				for (int i = 0; i < m_acceptTypes.size(); i++)
					acceptHeader[i] = m_acceptTypes.at(i).c_str();
			}

			// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpopenrequest
			wchar_t* acceptTypes = 
				m_acceptTypes.empty() ? (wchar_t*)Win32::WinHttp::DefaultAcceptTypes
				: (wchar_t*)&acceptHeader[0];
			//if (not m_acceptTypes.empty())
			//acceptTypes = (wchar_t*) &acceptHeader[0];
			WinHttpHandle hRequest = Win32::WinHttp::WinHttpOpenRequest(
				m_hConnect.Get(),
				verb.View().data(),
				path.c_str(),
				nullptr,
				reinterpret_cast<Win32::LPCWSTR>(Win32::WinHttp::NoReferer),
				reinterpret_cast<Win32::LPCWSTR*>(acceptTypes),
				Win32::WinHttp::FlagSecure
			);
			if (not hRequest)
				throw Error::Win32Error(Win32::GetLastError(), "WinHttpOpenRequest() failed");

			if (m_ignoreSslErrors)
			{
				DWORD flags =
					Win32::WinHttp::SecurityFlags::IgnoreUnknownCa |
					Win32::WinHttp::SecurityFlags::IgnoreCertWrongUsage |
					Win32::WinHttp::SecurityFlags::IgnoreCertCnInvalid |
					Win32::WinHttp::SecurityFlags::IgnoreCertDateInvalid;
				bool succeeded = Win32::WinHttp::WinHttpSetOption(
					hRequest.Get(),
					Win32::WinHttp::Options::SecurityFlags,
					&flags,
					sizeof(flags)
				);
				if (not succeeded)
					throw Error::Win32Error(Win32::GetLastError(), "WinHttpSetOption() failed");
			}

			// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpsendrequest
			bool succeeded = Win32::WinHttp::WinHttpSendRequest(
				hRequest.Get(),
				additionalHeaders.size() > 0 ? additionalHeaders.c_str() : (wchar_t*)Win32::WinHttp::NoAdditionalHeaders,
				-1L,
				requestBody.size() > 0 ? const_cast<char*>(&requestBody[0]) : (char*)Win32::WinHttp::NoRequestData,
				static_cast<Win32::DWORD>(requestBody.size()),
				static_cast<Win32::DWORD>(requestBody.size()),
				reinterpret_cast<Win32::DWORD_PTR>(this)
			);
			if (not succeeded)
				throw Error::Win32Error(Win32::GetLastError(), "WinHttpSendRequest() failed");

			// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpreceiveresponse
			succeeded = Win32::WinHttp::WinHttpReceiveResponse(hRequest.Get(), nullptr);
			if (not succeeded)
				throw Error::Win32Error(Win32::GetLastError(), "WinHttpReceiveResponse() failed");

			// Get the status code of the response
			Win32::DWORD statusCode = 0;
			Win32::DWORD statusCodeSize = sizeof(statusCode);
			// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpqueryheaders
			Win32::DWORD headerIndex = Win32::WinHttp::NoHeaderIndex;
			Win32::WinHttp::WinHttpQueryHeaders(
				hRequest.Get(),
				Win32::WinHttp::QueryStatusCode | Win32::WinHttp::QueryFlagNumber,
				(Win32::LPCWSTR)Win32::WinHttp::HeaderNameByIndex,
				&statusCode,
				&statusCodeSize,
				&headerIndex
			);

			Win32::DWORD bytesOfDataAvailable = 0;
			std::string responseBuffer(0, '\0');
			Win32::DWORD totalBytesRead = 0;
			// Used to be a do-while loop -- but MSVC seemed to be creating bad codegen
			// that would lead to response being cleared at the end of the loop!
			while(true)
			{
				// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpquerydataavailable
				if (not Win32::WinHttp::WinHttpQueryDataAvailable(hRequest.Get(), &bytesOfDataAvailable))
					throw Error::Win32Error(Win32::GetLastError(), "WinHttpQueryDataAvailable() failed");
				if (bytesOfDataAvailable == 0)
					break;

				// Allocate space for the buffer.
				responseBuffer.resize(responseBuffer.size() + bytesOfDataAvailable);
				Win32::DWORD downloadedBytes = 0;
				// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpreaddata
				// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpreaddataex
				bool succeeded = Win32::WinHttp::WinHttpReadData(
					hRequest.Get(),
					responseBuffer.data() + totalBytesRead,
					bytesOfDataAvailable,
					&downloadedBytes
				);
				if (not succeeded)
					throw Error::Win32Error(Win32::GetLastError(), "WinHttpQueryDataAvailable() failed");
				totalBytesRead += downloadedBytes;
			}

			return HttpRequestResult{ statusCode, std::move(responseBuffer) };
		}

		void Copy(const HttpWebClient& other)
		{
			m_userAgentName = other.m_userAgentName;
			m_serverToConnectTo = other.m_serverToConnectTo;
			m_port = other.m_port;
			m_ignoreSslErrors = other.m_ignoreSslErrors;
			m_acceptTypes = other.m_acceptTypes;
			m_additionalHeaders = other.m_additionalHeaders;
		}

		void Move(HttpWebClient& other)
		{
			m_userAgentName = std::move(other.m_userAgentName);
			m_serverToConnectTo = std::move(other.m_serverToConnectTo);
			m_port = other.m_port;
			m_ignoreSslErrors = other.m_ignoreSslErrors;
			m_acceptTypes = std::move(other.m_acceptTypes);
			m_additionalHeaders = std::move(other.m_additionalHeaders);
		}

		WinHttpHandle m_hSession;
		WinHttpHandle m_hConnect;
		std::wstring m_userAgentName;
		std::wstring m_serverToConnectTo;
		std::wstring m_proxy;
		unsigned m_port = 0;
		bool m_ignoreSslErrors = false;
		std::vector<std::wstring> m_acceptTypes;
		std::wstring m_additionalHeaders;
		ProxyInfo m_proxyInfo;
	};
}