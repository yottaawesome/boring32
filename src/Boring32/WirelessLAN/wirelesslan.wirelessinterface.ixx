export module boring32:wirelesslan.wirelessinterface;
import std;
import :win32;
import :util;
import :error;
import :wirelesslan.cleanup;

namespace Boring32::WirelessLAN
{
	template<typename T>
	T SimpleQueryInterface(
		Win32::HANDLE wlanHandle,
		const Win32::GUID& guid,
		const Win32::WLAN_INTF_OPCODE opcode
	)
	{
		if (not wlanHandle)
			throw Error::Boring32Error("wlanHandle cannot be null");
		if (opcode == wlan_intf_opcode_supported_infrastructure_auth_cipher_pairs)
			throw Error::Boring32Error("wlan_intf_opcode_supported_infrastructure_auth_cipher_pairs not supported by this function.");
		if (opcode == wlan_intf_opcode_supported_adhoc_auth_cipher_pairs)
			throw Error::Boring32Error("wlan_intf_opcode_supported_adhoc_auth_cipher_pairs not supported by this function.");

		// This will point to memory allocated by WLAN; we're responsible for freeing it.
		void* pWlanAllocatedMemory;
		Win32::WLAN_OPCODE_VALUE_TYPE opcodeType;
		Win32::DWORD dataSize = sizeof(T);
		// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlanqueryinterface
		Win32::DWORD status = Win32::WlanQueryInterface(
			wlanHandle,
			&guid,
			opcode,
			nullptr,
			&dataSize,
			&pWlanAllocatedMemory,
			&opcodeType
		);
		if (status != Win32::ErrorCodes::Success)
			throw Error::Win32Error(status, "WlanQueryInterface() failed");
		UniqueWLANMemory cleanup(pWlanAllocatedMemory);
		return T(*reinterpret_cast<T*>(pWlanAllocatedMemory));
	}

	template<>
	std::vector<Win32::DOT11_AUTH_CIPHER_PAIR> SimpleQueryInterface<std::vector<Win32::DOT11_AUTH_CIPHER_PAIR>>(
		Win32::HANDLE wlanHandle,
		const Win32::GUID& guid,
		const Win32::WLAN_INTF_OPCODE opcode
	)
	{
		if (not wlanHandle)
			throw Error::Boring32Error("wlanHandle cannot be null");
		if (opcode != wlan_intf_opcode_supported_infrastructure_auth_cipher_pairs)
			if (opcode != wlan_intf_opcode_supported_adhoc_auth_cipher_pairs)
				throw Error::Boring32Error("Must be one of wlan_intf_opcode_supported_infrastructure_auth_cipher_pairs or wlan_intf_opcode_supported_adhoc_auth_cipher_pairs.");

		// This will point to memory allocated by WLAN; we're responsible for freeing it.
		void* pWlanAllocatedMemory;
		Win32::WLAN_OPCODE_VALUE_TYPE opcodeType;
		Win32::DWORD dataSize = sizeof(Win32::WLAN_AUTH_CIPHER_PAIR_LIST);
		// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlanqueryinterface
		const Win32::DWORD status = Win32::WlanQueryInterface(
			wlanHandle,
			&guid,
			opcode,
			nullptr,
			&dataSize,
			&pWlanAllocatedMemory,
			&opcodeType
		);
		if (status != Win32::ErrorCodes::Success)
			throw Error::Win32Error(status, "WlanQueryInterface() failed");
		UniqueWLANMemory cleanup(pWlanAllocatedMemory);
		auto results = reinterpret_cast<Win32::WLAN_AUTH_CIPHER_PAIR_LIST*>(pWlanAllocatedMemory);
		std::vector<Win32::DOT11_AUTH_CIPHER_PAIR> returnVal;
		for (Win32::DWORD i = 0; i < results->dwNumberOfItems; i++)
			returnVal.push_back(results->pAuthCipherPairList[i]);

		return returnVal;
	}
}

export namespace Boring32::WirelessLAN
{
	// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/ne-wlanapi-wlan_interface_state-r1
	using InterfaceState = Win32::InterfaceState;

	struct WirelessInterface final
	{
		WirelessInterface(const WirelessInterface&) = default;
		WirelessInterface& operator=(const WirelessInterface&) = default;
		WirelessInterface(WirelessInterface&&) noexcept = default;
		WirelessInterface& operator=(WirelessInterface&&) noexcept = default;
		WirelessInterface(
			const SharedWLANHandle& wlanHandle,
			const Util::GloballyUniqueID& id,
			std::wstring description
		) : m_wlanHandle(wlanHandle),
			m_id(id),
			m_description(std::move(m_description))
		{ }

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
				Win32::WLAN_INTF_OPCODE::wlan_intf_opcode_interface_state
			);
		}

		Win32::WLAN_CONNECTION_ATTRIBUTES GetAttributes() const
		{
			// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/ns-wlanapi-wlan_connection_attributes
			return SimpleQueryInterface<Win32::WLAN_CONNECTION_ATTRIBUTES>(
				m_wlanHandle.get(),
				m_id.Get(),
				Win32::WLAN_INTF_OPCODE::wlan_intf_opcode_current_connection
			);
		}

		Win32::WLAN_STATISTICS GetStatistics() const
		{
			// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/ns-wlanapi-wlan_statistics
			return SimpleQueryInterface<Win32::WLAN_STATISTICS>(
				m_wlanHandle.get(),
				m_id.Get(),
				Win32::WLAN_INTF_OPCODE::wlan_intf_opcode_statistics
			);
		}

		bool IsAutoConfEnabled() const
		{
			return SimpleQueryInterface<bool>(
				m_wlanHandle.get(),
				m_id.Get(),
				Win32::WLAN_INTF_OPCODE::wlan_intf_opcode_autoconf_enabled
			);
		}

		Win32::DOT11_BSS_TYPE GetBSSType() const
		{
			// https://docs.microsoft.com/en-us/windows/win32/nativewifi/dot11-bss-type
			return SimpleQueryInterface<Win32::DOT11_BSS_TYPE>(
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
				Win32::WLAN_INTF_OPCODE::wlan_intf_opcode_background_scan_enabled
			);
		}

		Win32::ULONG GetChannelNumber() const
		{
			return SimpleQueryInterface<Win32::ULONG>(
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
				Win32::WLAN_INTF_OPCODE::wlan_intf_opcode_media_streaming_mode
			);
		}

		Win32::ULONG GetCurrentOperationMode() const
		{
			return SimpleQueryInterface<Win32::ULONG>(
				m_wlanHandle.get(),
				m_id.Get(),
				Win32::WLAN_INTF_OPCODE::wlan_intf_opcode_current_operation_mode
			);
		}

		bool IsSafeModeSupported() const
		{
			return SimpleQueryInterface<bool>(
				m_wlanHandle.get(),
				m_id.Get(),
				Win32::WLAN_INTF_OPCODE::wlan_intf_opcode_supported_safe_mode
			);
		}

		bool IsCertifiedSafeMode() const
		{
			return SimpleQueryInterface<bool>(
				m_wlanHandle.get(),
				m_id.Get(),
				Win32::WLAN_INTF_OPCODE::wlan_intf_opcode_certified_safe_mode
			);
		}

		std::vector<DOT11_AUTH_CIPHER_PAIR> GetInfrastructureCipherPairs() const
		{
			return SimpleQueryInterface<std::vector<Win32::DOT11_AUTH_CIPHER_PAIR>>(
				m_wlanHandle.get(),
				m_id.Get(),
				Win32::WLAN_INTF_OPCODE::wlan_intf_opcode_supported_adhoc_auth_cipher_pairs
			);
		}

		std::vector<Win32::DOT11_AUTH_CIPHER_PAIR> GetAdHocCipherPairs() const
		{
			return SimpleQueryInterface<std::vector<Win32::DOT11_AUTH_CIPHER_PAIR>>(
				m_wlanHandle.get(),
				m_id.Get(),
				Win32::WLAN_INTF_OPCODE::wlan_intf_opcode_supported_infrastructure_auth_cipher_pairs
			);
		}

		Win32::WLAN_INTERFACE_CAPABILITY GetCapability() const
		{
			// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/ns-wlanapi-wlan_interface_capability
			Win32::WLAN_INTERFACE_CAPABILITY* capability;
			// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlangetinterfacecapability
			const Win32::DWORD status = Win32::WlanGetInterfaceCapability(
				m_wlanHandle.get(),
				&m_id.Get(),
				nullptr,
				&capability
			);
			if (status != Win32::ErrorCodes::Success)
				throw Error::Win32Error(status, "WlanGetInterfaceCapability() failed");
			UniqueWLANMemory cleanup(capability);
			return *capability;
		}

	private:
		Util::GloballyUniqueID m_id;
		std::wstring m_description;
		SharedWLANHandle m_wlanHandle;
	};
}