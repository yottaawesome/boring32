export module boring32:winsock.winsockinit;
import :win32;
import :error;
import :winsock.winsockerror;

export namespace Boring32::WinSock
{
	class WinSockInit final
	{
	public:
		// Move semantics don't make sense for this object.
		~WinSockInit()
		{
			[[maybe_unused]] auto result = TryClose();
		}

		WinSockInit(const WinSockInit& other) = delete;
		auto operator=(const WinSockInit& other) -> WinSockInit& = delete;

		WinSockInit()
		{
			Initialize();
		}

		WinSockInit(Win32::DWORD highVersion, Win32::DWORD lowVersion)
			: m_highVersion(highVersion), m_lowVersion(lowVersion)
		{
			Initialize();
		}

		void Close()
		{
			auto result = TryClose();
			if (not result) 
				throw result.error();
		}

		auto TryClose() -> Error::Win32Expected
		{
			if (not m_lowVersion and not m_highVersion)
				return {};

			// https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsacleanup
			auto error = Win32::WinSock::WSACleanup();
			if (error)
				return std::unexpected(Error::Win32Error{ static_cast<Win32::DWORD>(error), "WSACleanup() failed", L"Ws2_32.dll" });
			m_lowVersion = 0;
			m_highVersion = 0;
			return {};
		}

		auto GetLowVersion() const noexcept -> Win32::DWORD
		{
			return m_lowVersion;
		}

		auto GetHighVersion() const noexcept -> Win32::DWORD
		{
			return m_highVersion;
		}

	private:
		void Initialize()
		{
			//https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsastartup
			auto error = Win32::WinSock::WSAStartup(Win32::MakeWord(m_highVersion, m_lowVersion), &m_wsaData);
			if (error) Error::ThrowNested(
				Error::Win32Error{static_cast<Win32::DWORD>(error), "WSAStartup() failed", L"Ws2_32.dll"},
				WinSockError("Failed to initialise WinSock")
			);
		}

		Win32::WinSock::WSAData m_wsaData;
		Win32::DWORD m_lowVersion = 2;
		Win32::DWORD m_highVersion = 2;
	};
}