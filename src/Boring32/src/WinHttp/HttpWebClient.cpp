#include "pch.hpp"
#include <stdexcept>
#include <vector>
#include <functional>
#include <sstream>
#include "include/WinHttp/HttpWebClient.hpp"

import boring32.error.win32error;

namespace Boring32::WinHttp
{
	HttpWebClient::~HttpWebClient() 
	{
		Close();
	}
	
	HttpWebClient::HttpWebClient()
	:	m_userAgentName(L""),
		m_serverToConnectTo(L""),
		m_port(0),
		m_ignoreSslErrors(false),
		m_acceptTypes({}),
		m_additionalHeaders(L"")
	{ }

	HttpWebClient::HttpWebClient(
		const std::wstring& userAgentName,
		const std::wstring& serverToConnectTo,
		const std::wstring& proxy,
		const UINT port,
		const bool ignoreSslErrors
	)
	:	m_userAgentName(userAgentName),
		m_serverToConnectTo(serverToConnectTo),
		m_port(port),
		m_ignoreSslErrors(ignoreSslErrors),
		m_proxy(proxy)
	{ }

	HttpWebClient::HttpWebClient(
		const std::wstring& userAgentName,
		const std::wstring& serverToConnectTo,
		const std::wstring& proxy,
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
		m_additionalHeaders(additionalHeaders),
		m_proxy(proxy)
	{ }

	HttpWebClient::HttpWebClient(const HttpWebClient& other)
	{
		Copy(other);
	}
	
	void HttpWebClient::operator=(const HttpWebClient& other)
	{
		Close();
		Copy(other);
	}

	void HttpWebClient::Copy(const HttpWebClient& other)
	{
		m_userAgentName = other.m_userAgentName;
		m_serverToConnectTo = other.m_serverToConnectTo;
		m_port = other.m_port;
		m_ignoreSslErrors = other.m_ignoreSslErrors;
		m_acceptTypes = other.m_acceptTypes;
		m_additionalHeaders = other.m_additionalHeaders;
	}

	HttpWebClient::HttpWebClient(HttpWebClient&& other) noexcept
	{
		Move(other);
	}

	void HttpWebClient::operator=(HttpWebClient&& other) noexcept
	{
		Close();
		Move(other);
		Connect();
	}

	void HttpWebClient::Move(HttpWebClient& other)
	{
		m_userAgentName = std::move(other.m_userAgentName);
		m_serverToConnectTo = std::move(other.m_serverToConnectTo);
		m_port = other.m_port;
		m_ignoreSslErrors = other.m_ignoreSslErrors;
		m_acceptTypes = std::move(other.m_acceptTypes);
		m_additionalHeaders = std::move(other.m_additionalHeaders);
	}

	void HttpWebClient::Close()
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

	void HttpWebClient::Connect()
	{
		const bool useAutomaticProxy = m_proxy.empty();

		m_hSession = WinHttpOpen(
			m_userAgentName.c_str(),
			useAutomaticProxy ? WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY : WINHTTP_ACCESS_TYPE_NO_PROXY,
			useAutomaticProxy ? WINHTTP_NO_PROXY_NAME : m_proxy.c_str(),
			WINHTTP_NO_PROXY_BYPASS,
			0
		);
		if (m_hSession == nullptr)
			throw Error::Win32Error("WinHttpOpen failed", GetLastError());

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
		if (m_hConnect == nullptr)
			throw Error::Win32Error("WinHttpConnect failed", GetLastError());
	}
	
	HttpRequestResult HttpWebClient::Get(const std::wstring& path)
	{
		return ExecuteRequest(L"GET", path, "", m_additionalHeaders);
	}

	HttpRequestResult HttpWebClient::Post(const std::wstring& path, const std::string& requestBody)
	{
		return ExecuteRequest(L"POST", path, requestBody, m_additionalHeaders);
	}

	HttpRequestResult HttpWebClient::Put(const std::wstring& path, const std::string& requestBody)
	{
		return ExecuteRequest(L"PUT", path, requestBody, m_additionalHeaders);
	}

	HttpRequestResult HttpWebClient::Delete(const std::wstring& path, const std::string& requestBody)
	{
		return ExecuteRequest(L"DELETE", path, requestBody, m_additionalHeaders);
	}

	HttpRequestResult HttpWebClient::ExecuteRequest(
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

		WinHttpHandle hRequest = WinHttpOpenRequest(
			m_hConnect.Get(),
			verb.c_str(),
			path.c_str(),
			nullptr,
			WINHTTP_NO_REFERER,
			m_acceptTypes.empty() 
				? WINHTTP_DEFAULT_ACCEPT_TYPES 
				: &acceptHeader[0],
			WINHTTP_FLAG_SECURE
		);
		if (hRequest == nullptr)
			throw Error::Win32Error("WinHttpOpenRequest failed", GetLastError());

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
				throw Error::Win32Error("WinHttpSetOption failed", GetLastError());
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
			(DWORD)requestBody.size(),
			(DWORD)requestBody.size(),
			reinterpret_cast<DWORD_PTR>(this)
		);
		if (succeeded == false)
			throw Error::Win32Error("WinHttpSendRequest failed", GetLastError());

		succeeded = WinHttpReceiveResponse(hRequest.Get(), nullptr);
		if (succeeded == false)
			throw Error::Win32Error("WinHttpReceiveResponse failed", GetLastError());

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
				throw Error::Win32Error("WinHttpQueryDataAvailable failed", GetLastError());
			if (dwSize == 0)
				continue;

			// Allocate space for the buffer.
			std::vector<char> outBuffer;
			outBuffer.resize(dwSize);
			DWORD dwDownloaded = 0;

			succeeded = WinHttpReadData(
				hRequest.Get(),
				(LPVOID)&outBuffer[0],
				dwSize,
				&dwDownloaded
			);
			if (succeeded == false)
				throw Error::Win32Error("WinHttpQueryDataAvailable failed", GetLastError());

			response.append(outBuffer.begin(), outBuffer.end());
		} while (dwSize > 0);

		return HttpRequestResult{ statusCode, response };
	}
}
