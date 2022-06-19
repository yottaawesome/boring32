module;

#include <string>
#include <Windows.h>
#include <wlanapi.h>

export module boring32.wirelesslan:interfaces;
import boring32.util;

export namespace Boring32::WirelessLAN
{
	enum class InterfaceState
	{
		not_ready = wlan_interface_state_not_ready,
		connected = wlan_interface_state_connected,
		ad_hoc_network_formed = wlan_interface_state_ad_hoc_network_formed,
		disconnecting = wlan_interface_state_disconnecting,
		disconnected = wlan_interface_state_disconnected,
		associating = wlan_interface_state_associating,
		discovering = wlan_interface_state_discovering,
		authenticating = wlan_interface_state_authenticating
	};

	class Interface
	{
		public:
			virtual ~Interface();
			Interface(
				const Util::GloballyUniqueID& id, 
				std::wstring description
			);

		public:
			const Util::GloballyUniqueID& GUID() const noexcept;
			const std::wstring& Description() const noexcept;

		protected:
			Util::GloballyUniqueID m_id;
			std::wstring m_description;
	};
}