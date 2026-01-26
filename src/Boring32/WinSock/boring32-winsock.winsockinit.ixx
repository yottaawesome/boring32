export module boring32:winsock.winsockinit;
import boring32.win32;
import :error;
import :winsock.winsockerror;

export namespace Boring32::WinSock
{
	struct WinSockInit final
	{
		// Move semantics don't make sense for this object.
		~WinSockInit()
		{
			Close(std::nothrow);
		}

		WinSockInit()
			: m_highVersion(2), m_lowVersion(2)
		{
			Initialize();
		}

		WinSockInit(const WinSockInit& other)
		{
			Copy(other);
		}

		WinSockInit(WinSockInit&&) noexcept = delete;

		WinSockInit(const Win32::DWORD highVersion, const Win32::DWORD lowVersion)
			: m_highVersion(highVersion), m_lowVersion(lowVersion)
		{
			Initialize();
		}

		WinSockInit& operator=(const WinSockInit& other)
		{
			return Copy(other);
		}

		WinSockInit& operator=(WinSockInit&&) noexcept = delete;

		void Close()
		{
			if (not m_lowVersion and not m_highVersion)
				return;

			// https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsacleanup
			const int error = Win32::WinSock::WSACleanup();
			if (error) Error::ThrowNested(
				Error::Win32Error(error, "WSACleanup() failed", L"Ws2_32.dll"),
				WinSockError("Failed to cleanup WinSock")
			);
			m_lowVersion = 0;
			m_highVersion = 0;
		}

		bool Close(std::nothrow_t) 
		try
		{
			Close();
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}

		Win32::DWORD GetLowVersion() const noexcept
		{
			return m_lowVersion;
		}

		Win32::DWORD GetHighVersion() const noexcept
		{
			return m_highVersion;
		}

		private:
		void Initialize()
		{
			//https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsastartup
			int error = Win32::WinSock::WSAStartup(Win32::MakeWord(m_highVersion, m_lowVersion), &m_wsaData);
			if (error) Error::ThrowNested(
				Error::Win32Error(error, "WSAStartup() failed", L"Ws2_32.dll"),
				WinSockError("Failed to initialise WinSock")
			);
		}

		WinSockInit& Copy(const WinSockInit& other)
		{
			if (&other == this)
				return *this;

			Close();
			if (other.m_highVersion or other.m_lowVersion)
			{
				m_highVersion = other.m_highVersion;
				m_lowVersion = other.m_lowVersion;
				Initialize();
			}

			return *this;
		}

		Win32::WinSock::WSAData m_wsaData;
		Win32::DWORD m_lowVersion;
		Win32::DWORD m_highVersion;
	};
}