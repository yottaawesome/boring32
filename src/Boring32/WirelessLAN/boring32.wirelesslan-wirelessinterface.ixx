export module boring32.wirelesslan:wirelessinterface;
import <string>;
import <vector>;
import <win32.hpp>;
import boring32.util;
import boring32.error;
import :cleanup;

namespace Boring32::WirelessLAN
{
	template<typename T>
	T SimpleQueryInterface(
		HANDLE wlanHandle,
		const GUID& guid,
		const WLAN_INTF_OPCODE opcode
	)
	{
		if (!wlanHandle)
			throw Error::Boring32Error("wlanHandle cannot be null");
		if (opcode == wlan_intf_opcode_supported_infrastructure_auth_cipher_pairs)
			throw Error::Boring32Error("wlan_intf_opcode_supported_infrastructure_auth_cipher_pairs not supported by this function.");
		if (opcode == wlan_intf_opcode_supported_adhoc_auth_cipher_pairs)
			throw Error::Boring32Error("wlan_intf_opcode_supported_adhoc_auth_cipher_pairs not supported by this function.");

		// This will point to memory allocated by WLAN; we're responsible for freeing it.
		void* pWlanAllocatedMemory;
		WLAN_OPCODE_VALUE_TYPE opcodeType;
		DWORD dataSize = sizeof(T);
		// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlanqueryinterface
		const DWORD status = WlanQueryInterface(
			wlanHandle,
			&guid,
			opcode,
			nullptr,
			&dataSize,
			&pWlanAllocatedMemory,
			&opcodeType
		);
		if (status != ERROR_SUCCESS)
			throw Error::Win32Error("WlanQueryInterface() failed", status);
		UniqueWLANMemory cleanup(pWlanAllocatedMemory);
		return T(*reinterpret_cast<T*>(pWlanAllocatedMemory));
	}

	template<>
	std::vector<DOT11_AUTH_CIPHER_PAIR> SimpleQueryInterface<std::vector<DOT11_AUTH_CIPHER_PAIR>>(
		HANDLE wlanHandle,
		const GUID& guid,
		const WLAN_INTF_OPCODE opcode
	)
	{
		if (!wlanHandle)
			throw Error::Boring32Error("wlanHandle cannot be null");
		if (opcode != wlan_intf_opcode_supported_infrastructure_auth_cipher_pairs)
			if (opcode != wlan_intf_opcode_supported_adhoc_auth_cipher_pairs)
				throw Error::Boring32Error("Must be one of wlan_intf_opcode_supported_infrastructure_auth_cipher_pairs or wlan_intf_opcode_supported_adhoc_auth_cipher_pairs.");

		// This will point to memory allocated by WLAN; we're responsible for freeing it.
		void* pWlanAllocatedMemory;
		WLAN_OPCODE_VALUE_TYPE opcodeType;
		DWORD dataSize = sizeof(WLAN_AUTH_CIPHER_PAIR_LIST);
		// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlanqueryinterface
		const DWORD status = WlanQueryInterface(
			wlanHandle,
			&guid,
			opcode,
			nullptr,
			&dataSize,
			&pWlanAllocatedMemory,
			&opcodeType
		);
		if (status != ERROR_SUCCESS)
			throw Error::Win32Error("WlanQueryInterface() failed", status);
		UniqueWLANMemory cleanup(pWlanAllocatedMemory);
		auto results = reinterpret_cast<WLAN_AUTH_CIPHER_PAIR_LIST*>(pWlanAllocatedMemory);
		std::vector<DOT11_AUTH_CIPHER_PAIR> returnVal;
		for (DWORD i = 0; i < results->dwNumberOfItems; i++)
			returnVal.push_back(results->pAuthCipherPairList[i]);

		return returnVal;
	}
}

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

	class WirelessInterface final
	{
		public:
			~WirelessInterface() = default;
			WirelessInterface(const WirelessInterface&) = default;
			WirelessInterface(WirelessInterface&&) noexcept = default;
			WirelessInterface(
				const SharedWLANHandle& wlanHandle,
				const Util::GloballyUniqueID& id,
				std::wstring description
			) : m_wlanHandle(wlanHandle),
				m_id(id),
				m_description(std::move(m_description))
			{ }

		public:
			WirelessInterface& operator=(const WirelessInterface&) = default;
			WirelessInterface& operator=(WirelessInterface&&) noexcept = default;

		public:
			const Util::GloballyUniqueID& GetGUID() const noexcept
			{
				return m_id;
			}

			const std::wstring& GetDescription() const noexcept
			{
				return m_description;
			}

			InterfaceState GetState() const
			{
				return SimpleQueryInterface<InterfaceState>(
					m_wlanHandle.get(),
					m_id.Get(),
					wlan_intf_opcode_interface_state
				);
			}

			WLAN_CONNECTION_ATTRIBUTES GetAttributes() const
			{
				// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/ns-wlanapi-wlan_connection_attributes
				return SimpleQueryInterface<WLAN_CONNECTION_ATTRIBUTES>(
					m_wlanHandle.get(),
					m_id.Get(),
					wlan_intf_opcode_current_connection
				);
			}

			WLAN_STATISTICS GetStatistics() const
			{
				// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/ns-wlanapi-wlan_statistics
				return SimpleQueryInterface<WLAN_STATISTICS>(
					m_wlanHandle.get(),
					m_id.Get(),
					wlan_intf_opcode_statistics
				);
			}

			bool IsAutoConfEnabled() const
			{
				return SimpleQueryInterface<bool>(
					m_wlanHandle.get(),
					m_id.Get(),
					wlan_intf_opcode_autoconf_enabled
				);
			}

			DOT11_BSS_TYPE GetBSSType() const
			{
				// https://docs.microsoft.com/en-us/windows/win32/nativewifi/dot11-bss-type
				return SimpleQueryInterface<DOT11_BSS_TYPE>(
					m_wlanHandle.get(),
					m_id.Get(),
					wlan_intf_opcode_bss_type
				);
			}

			bool IsBackgroundScanEnabled() const
			{
				return SimpleQueryInterface<bool>(
					m_wlanHandle.get(),
					m_id.Get(),
					wlan_intf_opcode_background_scan_enabled
				);
			}

			ULONG GetChannelNumber() const
			{
				return SimpleQueryInterface<ULONG>(
					m_wlanHandle.get(),
					m_id.Get(),
					wlan_intf_opcode_channel_number
				);
			}

			bool IsInStreamingMediaMode() const
			{
				return SimpleQueryInterface<bool>(
					m_wlanHandle.get(),
					m_id.Get(),
					wlan_intf_opcode_media_streaming_mode
				);
			}

			ULONG GetCurrentOperationMode() const
			{
				return SimpleQueryInterface<ULONG>(
					m_wlanHandle.get(),
					m_id.Get(),
					wlan_intf_opcode_current_operation_mode
				);
			}

			bool IsSafeModeSupported() const
			{
				return SimpleQueryInterface<bool>(
					m_wlanHandle.get(),
					m_id.Get(),
					wlan_intf_opcode_supported_safe_mode
				);
			}

			bool IsCertifiedSafeMode() const
			{
				return SimpleQueryInterface<bool>(
					m_wlanHandle.get(),
					m_id.Get(),
					wlan_intf_opcode_certified_safe_mode
				);
			}

			std::vector<DOT11_AUTH_CIPHER_PAIR> GetInfrastructureCipherPairs() const
			{
				return SimpleQueryInterface<std::vector<DOT11_AUTH_CIPHER_PAIR>>(
					m_wlanHandle.get(),
					m_id.Get(),
					wlan_intf_opcode_supported_adhoc_auth_cipher_pairs
				);
			}

			std::vector<DOT11_AUTH_CIPHER_PAIR> GetAdHocCipherPairs() const
			{
				return SimpleQueryInterface<std::vector<DOT11_AUTH_CIPHER_PAIR>>(
					m_wlanHandle.get(),
					m_id.Get(),
					wlan_intf_opcode_supported_infrastructure_auth_cipher_pairs
				);
			}

			WLAN_INTERFACE_CAPABILITY GetCapability() const
			{
				// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/ns-wlanapi-wlan_interface_capability
				WLAN_INTERFACE_CAPABILITY* capability;
				// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlangetinterfacecapability
				const DWORD status = WlanGetInterfaceCapability(
					m_wlanHandle.get(),
					&m_id.Get(),
					nullptr,
					&capability
				);
				if (status != ERROR_SUCCESS)
					throw Error::Win32Error("WlanGetInterfaceCapability() failed", status);
				UniqueWLANMemory cleanup(capability);
				return *capability;
			}

		private:
			Util::GloballyUniqueID m_id;
			std::wstring m_description;
			SharedWLANHandle m_wlanHandle;
	};
}