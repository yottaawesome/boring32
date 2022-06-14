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
		Close(std::nothrow);
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

	WinSockInit& WinSockInit::operator=(const WinSockInit& other)
	{
		return Copy(other);
	}

	void WinSockInit::Initialize()
	{
		//https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsastartup
		const int error = WSAStartup(MAKEWORD(m_highVersion, m_lowVersion), &m_wsaData);
		if (error) Error::ThrowNested(
			Error::Win32Error(std::source_location::current(), "WSAStartup() failed", error, L"Ws2_32.dll"),
			WinSockError("Failed to initialise WinSock")
		);
	}
	
	void WinSockInit::Close()
	{
		if (!m_lowVersion && !m_highVersion)
			return;

		// https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsacleanup
		const int error = WSACleanup();
		if (error) Error::ThrowNested(
			Error::Win32Error(std::source_location::current(), "WSACleanup() failed", error, L"Ws2_32.dll"),
			WinSockError("Failed to cleanup WinSock")
		);
		m_lowVersion = 0;
		m_highVersion = 0;
	}

	bool WinSockInit::Close(const std::nothrow_t&) try
	{
		Close();
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}

	WinSockInit& WinSockInit::Copy(const WinSockInit& other)
	{
		if (&other == this)
			return *this;

		Close();
		if (other.m_highVersion || other.m_lowVersion)
		{
			m_highVersion = other.m_highVersion;
			m_lowVersion = other.m_lowVersion;
			Initialize();
		}

		return *this;
	}

	DWORD WinSockInit::GetLowVersion() const noexcept
	{
		return m_lowVersion;
	}

	DWORD WinSockInit::GetHighVersion() const noexcept
	{
		return m_highVersion;
	}
}