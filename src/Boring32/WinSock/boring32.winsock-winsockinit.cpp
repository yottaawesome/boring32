module;

#include <stdexcept>
#include <source_location>
#include <windows.h>
#include <winsock2.h>

module boring32.winsock:winsockinit;
import boring32.error;
import :winsockerror;

namespace Boring32::WinSock
{
	WinSockInit::~WinSockInit()
	{
		Close();
	}
	
	WinSockInit::WinSockInit() : m_highVersion(2), m_lowVersion(2)
	{
		Initialize();
	}

	WinSockInit::WinSockInit(const WinSockInit& other)
	{
		Copy(other);
	}

	WinSockInit::WinSockInit(const DWORD highVersion, const DWORD lowVersion)
		: m_highVersion(highVersion), m_lowVersion(lowVersion)
	{
		Initialize();
	}

	void WinSockInit::Initialize()
	{
		//https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsastartup
		const int error = WSAStartup(MAKEWORD(m_highVersion, m_lowVersion), &m_wsaData);
		if (error) Error::ThrowNested(
			Error::Win32Error(std::source_location::current(), "WSAStartup() failed", error, L"Ws2_32.dll"),
			WinSockError(std::source_location::current(), "Failed to initialise WinSock")
		);
	}

	const WSAData& WinSockInit::GetData() const noexcept
	{
		//https://docs.microsoft.com/en-us/windows/win32/api/winsock/ns-winsock-wsadata
		return m_wsaData;
	}
	
	void WinSockInit::Close()
	{
		WSACleanup();
		m_lowVersion = 0;
		m_highVersion = 0;
	}

	WinSockInit& WinSockInit::Copy(const WinSockInit& other)
	{
		if (&other == this)
			return *this;
		Close();
		if (other.m_highVersion && other.m_lowVersion)
		{
			m_highVersion = other.m_highVersion;
			m_lowVersion = other.m_lowVersion;
			Initialize();
		}

		return *this;
	}
}