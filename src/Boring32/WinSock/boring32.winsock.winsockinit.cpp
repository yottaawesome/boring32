module;

#include <stdexcept>
#include <source_location>
#include <windows.h>
#include <winsock2.h>

module boring32.winsock.winsockinit;
import boring32.winsock.winsockerror;

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
		if (const int err = WSAStartup(MAKEWORD(highVersion, lowVersion), &m_wsaData))
			throw WinSockError(std::source_location::current(), "Failed to initialise WinSock", err);
	}

	const WSAData& WinSockInit::GetData() const noexcept
	{
		//https://docs.microsoft.com/en-us/windows/win32/api/winsock/ns-winsock-wsadata
		return m_wsaData;
	}
}