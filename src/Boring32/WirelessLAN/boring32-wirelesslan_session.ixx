export module boring32:wirelesslan_session;
import boring32.shared;
import :error;
import :wirelesslan_cleanup;
import :wirelesslan_wirelessinterfaces;

export namespace Boring32::WirelessLAN
{
	struct Session final
	{
		WirelessInterfaces Interfaces;

		Session()
			: Interfaces(Open())
		{}

		Session(const Session& other) = default;
		Session(Session&& other) noexcept = default;

		Session& operator=(const Session& other) = default;
		Session& operator=(Session&& other) noexcept = default;

		void Close()
		{
			m_wlanHandle = nullptr;
			m_maxClientVersion = 0;
			m_negotiatedVersion = 0;
		}

		Win32::DWORD GetMaxClientVersion() const noexcept
		{
			return m_maxClientVersion;
		}

		Win32::DWORD GetNegotiatedVersion() const noexcept
		{
			return m_negotiatedVersion;
		}

		private:
		SharedWLANHandle Open()
		{
			if (m_wlanHandle)
				return m_wlanHandle;

			// Open a WLAN session
			Win32::HANDLE wlanHandle;
			Win32::DWORD status = Win32::WlanOpenHandle(
				m_maxClientVersion,
				nullptr,
				&m_negotiatedVersion,
				&wlanHandle
			);
			if (status != Win32::ErrorCodes::Success)
				throw Boring32::Error::Win32Error(status, "WlanOpenHandle() failed");
			m_wlanHandle = CreateSharedWLANHandle(wlanHandle);
			return m_wlanHandle;
		}
			
		SharedWLANHandle m_wlanHandle;
		Win32::DWORD m_maxClientVersion = 2;
		Win32::DWORD m_negotiatedVersion = 0;
	};
}