export module boring32:winsock_winsockinit;
import boring32.error;
import boring32.shared;
import :winsock_winsockerror;

export namespace Boring32::WinSock
{
	class WinSockInit
	{
		// Move semantics don't make sense for this object.
		public:
			virtual ~WinSockInit()
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

		public:
			virtual WinSockInit& operator=(const WinSockInit& other)
			{
				return Copy(other);
			}

			virtual WinSockInit& operator=(WinSockInit&&) noexcept = delete;

		public:
			virtual void Close()
			{
				if (!m_lowVersion && !m_highVersion)
					return;

				// https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsacleanup
				const int error = Win32::WinSock::WSACleanup();
				if (error) Error::ThrowNested(
					Error::Win32Error("WSACleanup() failed", error, L"Ws2_32.dll"),
					WinSockError("Failed to cleanup WinSock")
				);
				m_lowVersion = 0;
				m_highVersion = 0;
			}

			virtual bool Close(
				const std::nothrow_t&
			) try
			{
				Close();
				return true;
			}
			catch (const std::exception&)
			{
				return false;
			}

			virtual Win32::DWORD GetLowVersion() const noexcept
			{
				return m_lowVersion;
			}

			virtual Win32::DWORD GetHighVersion() const noexcept
			{
				return m_highVersion;
			}

		protected:
			virtual void Initialize()
			{
				//https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsastartup
				const int error = Win32::WinSock::WSAStartup(Win32::MakeWord(m_highVersion, m_lowVersion), &m_wsaData);
				if (error) Error::ThrowNested(
					Error::Win32Error("WSAStartup() failed", error, L"Ws2_32.dll"),
					WinSockError("Failed to initialise WinSock")
				);
			}

			virtual WinSockInit& Copy(const WinSockInit& other)
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

		protected:
			Win32::WinSock::WSAData m_wsaData;
			Win32::DWORD m_lowVersion;
			Win32::DWORD m_highVersion;
	};
}