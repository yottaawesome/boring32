export module boring32.wirelesslan:wirelessinterfaces;
import <iostream>;
import <format>;
import <vector>;
import <win32.hpp>;
import boring32.error;
import :cleanup;
import :wirelessinterface;

export namespace Boring32::WirelessLAN
{
	class WirelessInterfaces final
	{
		public:
			WirelessInterfaces(SharedWLANHandle session)
				: m_session(session)
			{}

		public:
			std::vector<WirelessInterface> GetAll() const
			{
				// Enumerate the interface list
				PWLAN_INTERFACE_INFO_LIST pIfList = nullptr;
				// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlanenuminterfaces
				DWORD status = WlanEnumInterfaces(m_session.get(), nullptr, &pIfList);
				if (status != ERROR_SUCCESS)
					throw Error::Win32Error("WlanEnumInterfaces() failed", status);
				UniqueWLANMemory interfaceList = UniqueWLANMemory(pIfList);

				std::vector<WirelessInterface> returnVal;
				for (DWORD index = 0; index < pIfList->dwNumberOfItems; index++)
				{
					// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/ns-wlanapi-wlan_interface_info
					PWLAN_INTERFACE_INFO interfaceInfo = &pIfList->InterfaceInfo[index];
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