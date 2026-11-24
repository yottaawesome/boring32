export module boring32:wirelesslan.wirelessinterfaces;
import std;
import :win32;
import :error;
import :wirelesslan.cleanup;
import :wirelesslan.wirelessinterface;

export namespace Boring32::WirelessLAN
{
	class WirelessInterfaces final
	{
	public:
		WirelessInterfaces(SharedWLANHandle session)
			: m_session(session)
		{}

		auto GetAll() const -> std::vector<WirelessInterface>
		{
			// Enumerate the interface list
			auto pIfList = Win32::PWLAN_INTERFACE_INFO_LIST{};
			// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlanenuminterfaces
			auto status = Win32::DWORD{ Win32::WlanEnumInterfaces(m_session.get(), nullptr, &pIfList) };
			if (status != Win32::ErrorCodes::Success)
				throw Error::Win32Error{status, "WlanEnumInterfaces() failed"};
			auto interfaceList = UniqueWLANMemory{ pIfList };

			auto returnVal = std::vector<WirelessInterface>{};
			for (auto index = Win32::DWORD{}; index < pIfList->dwNumberOfItems; index++)
			{
				// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/ns-wlanapi-wlan_interface_info
				auto interfaceInfo = Win32::PWLAN_INTERFACE_INFO{ &pIfList->InterfaceInfo[index] };
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