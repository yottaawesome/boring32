export module boring32:wirelesslan_wirelessinterfaces;
import boring32.shared;
import :error;
import :wirelesslan_cleanup;
import :wirelesslan_wirelessinterface;

export namespace Boring32::WirelessLAN
{
	struct WirelessInterfaces final
	{
		WirelessInterfaces(SharedWLANHandle session)
			: m_session(session)
		{}

		std::vector<WirelessInterface> GetAll() const
		{
			// Enumerate the interface list
			Win32::PWLAN_INTERFACE_INFO_LIST pIfList = nullptr;
			// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlanenuminterfaces
			Win32::DWORD status = Win32::WlanEnumInterfaces(m_session.get(), nullptr, &pIfList);
			if (status != Win32::ErrorCodes::Success)
				throw Error::Win32Error(status, "WlanEnumInterfaces() failed");
			UniqueWLANMemory interfaceList = UniqueWLANMemory(pIfList);

			std::vector<WirelessInterface> returnVal;
			for (Win32::DWORD index = 0; index < pIfList->dwNumberOfItems; index++)
			{
				// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/ns-wlanapi-wlan_interface_info
				Win32::PWLAN_INTERFACE_INFO interfaceInfo = &pIfList->InterfaceInfo[index];
				returnVal.emplace_back(
					m_session,
					interfaceInfo->InterfaceGuid,
					interfaceInfo->strInterfaceDescription
				);
			}
			return returnVal;
		}

		private:
		SharedWLANHandle m_session;
	};
}