export module boring32.wirelesslan:session;
import <win32.hpp>;
import <format>;
import <iostream>;
import boring32.error;
import :cleanup;
import :wirelessinterfaces;

export namespace Boring32::WirelessLAN
{
	class Session final
	{
		public:
			~Session() = default; 
			Session()
				: Interfaces(Open())
			{}

			Session(const Session& other) = default;
			Session(Session&& other) noexcept = default;

		public:
			Session& operator=(const Session& other) = default;
			Session& operator=(Session&& other) noexcept = default;

		public:
			void Close()
			{
				m_wlanHandle = nullptr;
				m_maxClientVersion = 0;
				m_negotiatedVersion = 0;
			}

			DWORD GetMaxClientVersion() const noexcept
			{
				return m_maxClientVersion;
			}

			DWORD GetNegotiatedVersion() const noexcept
			{
				return m_negotiatedVersion;
			}
			//virtual WirelessInterfaces Interfaces();

		private:
			SharedWLANHandle Open()
			{
				if (m_wlanHandle)
					return m_wlanHandle;

				// Open a WLAN session
				HANDLE wlanHandle;
				const DWORD status = WlanOpenHandle(
					m_maxClientVersion,
					nullptr,
					&m_negotiatedVersion,
					&wlanHandle
				);
				if (status != ERROR_SUCCESS)
					throw Boring32::Error::Win32Error("WlanOpenHandle() failed", status);
				m_wlanHandle = CreateSharedWLANHandle(wlanHandle);
				return m_wlanHandle;
			}
			
		private:
			SharedWLANHandle m_wlanHandle;
			DWORD m_maxClientVersion = 2;
			DWORD m_negotiatedVersion = 0;

			// declared here due to initialisation order
			// may need to reconsider this approach
		public:
			WirelessInterfaces Interfaces;
	};
}