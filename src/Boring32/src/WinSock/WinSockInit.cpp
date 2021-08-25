module;

#include "pch.hpp"

module boring32.winsock;

namespace Boring32::WinSock
{
	WinSockInit::~WinSockInit()
	{
		WSACleanup();
	}
	
	WinSockInit::WinSockInit()
	{
		Initialize(2, 2);
	}

	WinSockInit::WinSockInit(const DWORD highVersion, const DWORD lowVersion)
	{
		Initialize(highVersion, lowVersion);
	}

	void WinSockInit::Initialize(const DWORD highVersion, const DWORD lowVersion)
	{
		//https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsastartup
		const int err = WSAStartup(MAKEWORD(highVersion, lowVersion), &m_wsaData);
		if (err != 0)
		{
			throw std::runtime_error(__FUNCSIG__ ": Failed to initialise WinSock: " + err);
		}
	}

	const WSAData& WinSockInit::GetData() const noexcept
	{
		//https://docs.microsoft.com/en-us/windows/win32/api/winsock/ns-winsock-wsadata
		return m_wsaData;
	}
}