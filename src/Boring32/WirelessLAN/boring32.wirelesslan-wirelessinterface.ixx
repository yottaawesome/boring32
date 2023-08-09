export module boring32.wirelesslan:wirelessinterface;
import boring32.util;
import :cleanup;
import std;

import <win32.hpp>;

export namespace Boring32::WirelessLAN
{
	// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/ne-wlanapi-wlan_interface_state-r1
	enum class InterfaceState : int
	{
		NotReady = wlan_interface_state_not_ready,
		Connected = wlan_interface_state_connected,
		AdHocNetworkFormed = wlan_interface_state_ad_hoc_network_formed,
		Disconnecting = wlan_interface_state_disconnecting,
		Disconnected = wlan_interface_state_disconnected,
		Associating = wlan_interface_state_associating,
		Discovering = wlan_interface_state_discovering,
		Authenticating = wlan_interface_state_authenticating
	};

	class WirelessInterface
	{
		public:
			virtual ~WirelessInterface() = default;
			WirelessInterface(const WirelessInterface&) = default;
			WirelessInterface(WirelessInterface&&) noexcept = default;
			WirelessInterface(
				const SharedWLANHandle& wlanHandle,
				const Util::GloballyUniqueID& id,
				std::wstring description
			);

		public:
			virtual WirelessInterface& operator=(const WirelessInterface&) = default;
			virtual WirelessInterface& operator=(WirelessInterface&&) noexcept = default;

		public:
			[[nodiscard]]
			const Util::GloballyUniqueID& GetGUID() const noexcept;
			[[nodiscard]]
			const std::wstring& GetDescription() const noexcept;
			[[nodiscard]]
			virtual InterfaceState GetState() const;
			[[nodiscard]]
			virtual WLAN_CONNECTION_ATTRIBUTES GetAttributes() const;
			[[nodiscard]]
			virtual WLAN_INTERFACE_CAPABILITY GetCapability() const;
			[[nodiscard]]
			virtual WLAN_STATISTICS GetStatistics() const;
			[[nodiscard]]
			virtual bool IsAutoConfEnabled() const;
			[[nodiscard]]
			virtual DOT11_BSS_TYPE GetBSSType() const;
			[[nodiscard]]
			virtual bool IsBackgroundScanEnabled() const;
			[[nodiscard]]
			virtual ULONG GetChannelNumber() const;
			[[nodiscard]]
			virtual bool IsInStreamingMediaMode() const;
			[[nodiscard]]
			virtual ULONG GetCurrentOperationMode() const;
			[[nodiscard]]
			virtual bool IsSafeModeSupported() const;
			[[nodiscard]]
			virtual bool IsCertifiedSafeMode() const;
			[[nodiscard]]
			virtual std::vector<DOT11_AUTH_CIPHER_PAIR> GetInfrastructureCipherPairs() const;
			[[nodiscard]]
			virtual std::vector<DOT11_AUTH_CIPHER_PAIR> GetAdHocCipherPairs() const;

		protected:
			Util::GloballyUniqueID m_id;
			std::wstring m_description;
			SharedWLANHandle m_wlanHandle;
	};
}