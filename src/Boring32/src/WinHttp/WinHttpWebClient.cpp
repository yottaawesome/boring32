#include "pch.hpp"
#include <stdexcept>
#include <vector>
#include <functional>
#include <sstream>
#include "include/WinHttp/WinHttpWebClient.hpp"

namespace Boring32::WinHttp
{
	WinHttpWebClient::~WinHttpWebClient() 
	{
		Close();
	}
	
	WinHttpWebClient::WinHttpWebClient()
	:	m_userAgentName(L""),
		m_serverToConnectTo(L""),
		m_port(0),
		m_ignoreSslErrors(false),
		m_acceptTypes({}),
		m_additionalHeaders(L"")
	{ }

	WinHttpWebClient::WinHttpWebClient(
		const std::wstring& userAgentName,
		const std::wstring& serverToConnectTo,
		const UINT port,
		const bool ignoreSslErrors,
		const std::vector<std::wstring>& acceptTypes,
		const std::wstring& additionalHeaders
	)
	:	m_userAgentName(userAgentName),
		m_serverToConnectTo(serverToConnectTo),
		m_port(port),
		m_ignoreSslErrors(ignoreSslErrors),
		m_acceptTypes(acceptTypes),
		m_additionalHeaders(additionalHeaders)
	{
	}

	WinHttpWebClient::WinHttpWebClient(const WinHttpWebClient& other)
	{
		Copy(other);
		Connect();
	}
	
	void WinHttpWebClient::operator=(const WinHttpWebClient& other)
	{
		Close();
		Copy(other);
		Connect();
	}

	void WinHttpWebClient::Copy(const WinHttpWebClient& other)
	{
		m_userAgentName = other.m_userAgentName;
		m_serverToConnectTo = other.m_serverToConnectTo;
		m_port = other.m_port;
		m_ignoreSslErrors = other.m_ignoreSslErrors;
		m_acceptTypes = other.m_acceptTypes;
		m_additionalHeaders = other.m_additionalHeaders;
	}

	WinHttpWebClient::WinHttpWebClient(WinHttpWebClient&& other) noexcept
	{
		Move(other);
		Connect();
	}

	void WinHttpWebClient::operator=(WinHttpWebClient&& other) noexcept
	{
		Close();
		Move(other);
		Connect();
	}

	void WinHttpWebClient::Move(WinHttpWebClient& other)
	{
		m_userAgentName = std::move(other.m_userAgentName);
		m_serverToConnectTo = std::move(other.m_serverToConnectTo);
		m_port = other.m_port;
		m_ignoreSslErrors = other.m_ignoreSslErrors;
		m_acceptTypes = std::move(other.m_acceptTypes);
		m_additionalHeaders = std::move(other.m_additionalHeaders);
	}

	void WinHttpWebClient::Close()
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

	void WinHttpWebClient::Connect()
	{
		m_hSession = WinHttpOpen(
			m_userAgentName.c_str(),
			WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
			WINHTTP_NO_PROXY_NAME,
			WINHTTP_NO_PROXY_BYPASS,
			0
		);
		if (m_hSession == nullptr)
			throw std::runtime_error("WinHttpOpen failed");

		m_hConnect = WinHttpConnect(
			m_hSession.Get(),
			m_serverToConnectTo.c_str(),
			m_port,
			0);
		if (m_hConnect == nullptr)
			throw std::runtime_error("WinHttpConnect failed");
	}
	
	void WinHttpWebClient::Get(const std::wstring& path)
	{
		ExecuteRequest(L"GET", path, "", m_additionalHeaders);
	}

	void WinHttpWebClient::Post(const std::wstring& path, const std::string& requestBody)
	{
		ExecuteRequest(L"POST", path, requestBody, m_additionalHeaders);
	}

	void WinHttpWebClient::Put(const std::wstring& path, const std::string& requestBody)
	{
		ExecuteRequest(L"PUT", path, requestBody, m_additionalHeaders);
	}

	void WinHttpWebClient::Delete(const std::wstring& path, const std::string& requestBody)
	{
		ExecuteRequest(L"DELETE", path, requestBody, m_additionalHeaders);
	}

	WinHttpRequestResult WinHttpWebClient::ExecuteRequest(
		const std::wstring& verb, 
		const std::wstring& path,
		const std::string& requestBody,
		const std::wstring& additionalHeaders
	)
	{
		std::unique_ptr<LPCWSTR[]> acceptHeader = nullptr;
		if (m_acceptTypes.size() > 0)
		{
			// acceptHeader must be a null terminated array
			acceptHeader = std::unique_ptr<LPCWSTR[]>(new LPCWSTR[m_acceptTypes.size()+1]);
			for (int i = 0; i < m_acceptTypes.size(); i++)
				acceptHeader[i] = m_acceptTypes.at(i).c_str();
			acceptHeader[m_acceptTypes.size()] = nullptr;
		}

		WinHttpHandle hRequest = WinHttpOpenRequest(
			m_hConnect.Get(),
			verb.c_str(),
			path.c_str(),
			nullptr,
			WINHTTP_NO_REFERER,
			acceptHeader != nullptr 
				? acceptHeader.get() 
				: WINHTTP_DEFAULT_ACCEPT_TYPES,
			WINHTTP_FLAG_SECURE
		);
		if (hRequest == nullptr)
			throw std::runtime_error("WinHttpOpenRequest failed");

		if (m_ignoreSslErrors)
		{
			DWORD dwFlags =
				SECURITY_FLAG_IGNORE_UNKNOWN_CA |
				SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE |
				SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
				SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
			bool succeeded = WinHttpSetOption(
				hRequest.Get(),
				WINHTTP_OPTION_SECURITY_FLAGS,
				&dwFlags,
				sizeof(dwFlags)
			);
			if(succeeded == false)
				throw std::runtime_error("WinHttpOpenRequest failed to set SSL options");
		}

		bool succeeded = WinHttpSendRequest(
			hRequest.Get(),
			additionalHeaders.size() > 0 
				? additionalHeaders.c_str()
				: WINHTTP_NO_ADDITIONAL_HEADERS,
			-1L,
			requestBody.size() > 0 
				? (LPVOID)requestBody.c_str()
				: WINHTTP_NO_REQUEST_DATA,
			requestBody.size(),
			requestBody.size(),
			reinterpret_cast<DWORD_PTR>(this)
		);
		if (succeeded == false)
			throw std::runtime_error("WinHttpSendRequest failed");

		succeeded = WinHttpReceiveResponse(hRequest.Get(), nullptr);
		if (!succeeded)
			throw std::runtime_error("WinHttpReceiveResponse failed");

		DWORD statusCode = 0;
		DWORD statusCodeSize = sizeof(statusCode);
		WinHttpQueryHeaders(
			hRequest.Get(),
			WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
			WINHTTP_HEADER_NAME_BY_INDEX,
			&statusCode,
			&statusCodeSize,
			WINHTTP_NO_HEADER_INDEX);

		DWORD dwSize = 0;
		std::string response = "";
		do
		{
			bool succeeded = WinHttpQueryDataAvailable(hRequest.Get(), &dwSize);
			if (succeeded == false)
				throw std::runtime_error("WinHttpQueryDataAvailable failed");

			// Allocate space for the buffer.
			std::vector<char> outBuffer(dwSize + 1, 0);
			DWORD dwDownloaded = 0;

			succeeded = WinHttpReadData(
				hRequest.Get(),
				(LPVOID)&outBuffer[0],
				dwSize,
				&dwDownloaded
			);
			if (succeeded == false)
				throw std::runtime_error("WinHttpReadData failed");

			response.append(outBuffer.begin(), outBuffer.end());
		} while (dwSize > 0);

		return WinHttpRequestResult{ statusCode, response };
	}
}
