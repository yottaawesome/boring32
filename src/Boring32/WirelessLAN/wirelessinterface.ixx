export module boring32:wirelesslan.wirelessinterface;
import std;
import :win32;
import :util;
import :error;
import :wirelesslan.cleanup;

namespace Boring32::WirelessLAN
{
	template<typename T>
	auto SimpleQueryInterface(
		Win32::HANDLE wlanHandle,
		const Win32::GUID& guid,
		Win32::WLAN_INTF_OPCODE opcode
	) -> T
	{
		if (not wlanHandle)
			throw Error::Boring32Error{ "wlanHandle cannot be null" };
		if (opcode == wlan_intf_opcode_supported_infrastructure_auth_cipher_pairs)
			throw Error::Boring32Error{ "wlan_intf_opcode_supported_infrastructure_auth_cipher_pairs not supported by this function." };
		if (opcode == wlan_intf_opcode_supported_adhoc_auth_cipher_pairs)
			throw Error::Boring32Error{ "wlan_intf_opcode_supported_adhoc_auth_cipher_pairs not supported by this function." };

		// This will point to memory allocated by WLAN; we're responsible for freeing it.
		auto pWlanAllocatedMemory = static_cast<void*>(nullptr);
		auto opcodeType = Win32::WLAN_OPCODE_VALUE_TYPE{};
		auto dataSize = Win32::DWORD{ sizeof(T) };
		// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlanqueryinterface
		auto status = Win32::DWORD{ 
			Win32::WlanQueryInterface(
				wlanHandle,
				&guid,
				opcode,
				nullptr,
				&dataSize,
				&pWlanAllocatedMemory,
				&opcodeType
			)};
		if (status != Win32::ErrorCodes::Success)
			throw Error::Win32Error{status, "WlanQueryInterface() failed"};
		auto cleanup = UniqueWLANMemory{pWlanAllocatedMemory};
		return T(*reinterpret_cast<T*>(pWlanAllocatedMemory));
	}

	template<>
	auto SimpleQueryInterface<std::vector<Win32::DOT11_AUTH_CIPHER_PAIR>>(
		Win32::HANDLE wlanHandle,
		const Win32::GUID& guid,
		Win32::WLAN_INTF_OPCODE opcode
	) -> std::vector<Win32::DOT11_AUTH_CIPHER_PAIR>
	{
		if (not wlanHandle)
			throw Error::Boring32Error{ "wlanHandle cannot be null" };
		if (opcode != wlan_intf_opcode_supported_infrastructure_auth_cipher_pairs)
			if (opcode != wlan_intf_opcode_supported_adhoc_auth_cipher_pairs)
				throw Error::Boring32Error{ "Must be one of wlan_intf_opcode_supported_infrastructure_auth_cipher_pairs or wlan_intf_opcode_supported_adhoc_auth_cipher_pairs." };

		// This will point to memory allocated by WLAN; we're responsible for freeing it.
		auto pWlanAllocatedMemory = static_cast<void*>(nullptr);
		auto opcodeType = Win32::WLAN_OPCODE_VALUE_TYPE{};
		auto dataSize = Win32::DWORD{ sizeof(Win32::WLAN_AUTH_CIPHER_PAIR_LIST) };
		// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlanqueryinterface
		auto status = Win32::DWORD{ 
			Win32::WlanQueryInterface(
				wlanHandle,
				&guid,
				opcode,
				nullptr,
				&dataSize,
				&pWlanAllocatedMemory,
				&opcodeType
			)};
		if (status != Win32::ErrorCodes::Success)
			throw Error::Win32Error{status, "WlanQueryInterface() failed"};
		auto cleanup = UniqueWLANMemory{pWlanAllocatedMemory};
		auto results = reinterpret_cast<Win32::WLAN_AUTH_CIPHER_PAIR_LIST*>(pWlanAllocatedMemory);
		auto returnVal = std::vector<Win32::DOT11_AUTH_CIPHER_PAIR>{};
		for (auto i = Win32::DWORD{}; i < results->dwNumberOfItems; i++)
			returnVal.push_back(results->pAuthCipherPairList[i]);

		return returnVal;
	}
}

export namespace Boring32::WirelessLAN
{
	// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/ne-wlanapi-wlan_interface_state-r1
	using InterfaceState = Win32::InterfaceState;

	class WirelessInterface final
	{
	public:
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

		auto GetGUID() const noexcept -> Util::GloballyUniqueID
		{
			return m_id;
		}

		auto GetDescription() const noexcept -> std::wstring
		{
			return m_description;
		}

		auto GetState() const -> InterfaceState
		{
			return SimpleQueryInterface<InterfaceState>(
				m_wlanHandle.get(),
				m_id.Get(),
				Win32::WLAN_INTF_OPCODE::wlan_intf_opcode_interface_state
			);
		}

		auto GetAttributes() const -> Win32::WLAN_CONNECTION_ATTRIBUTES
		{
			// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/ns-wlanapi-wlan_connection_attributes
			return SimpleQueryInterface<Win32::WLAN_CONNECTION_ATTRIBUTES>(
				m_wlanHandle.get(),
				m_id.Get(),
				Win32::WLAN_INTF_OPCODE::wlan_intf_opcode_current_connection
			);
		}

		auto GetStatistics() const -> Win32::WLAN_STATISTICS
		{
			// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/ns-wlanapi-wlan_statistics
			return SimpleQueryInterface<Win32::WLAN_STATISTICS>(
				m_wlanHandle.get(),
				m_id.Get(),
				Win32::WLAN_INTF_OPCODE::wlan_intf_opcode_statistics
			);
		}

		auto IsAutoConfEnabled() const -> bool
		{
			return SimpleQueryInterface<bool>(
				m_wlanHandle.get(),
				m_id.Get(),
				Win32::WLAN_INTF_OPCODE::wlan_intf_opcode_autoconf_enabled
			);
		}

		auto GetBSSType() const -> Win32::DOT11_BSS_TYPE
		{
			// https://docs.microsoft.com/en-us/windows/win32/nativewifi/dot11-bss-type
			return SimpleQueryInterface<Win32::DOT11_BSS_TYPE>(
				m_wlanHandle.get(),
				m_id.Get(),
				wlan_intf_opcode_bss_type
			);
		}

		auto IsBackgroundScanEnabled() const -> bool
		{
			return SimpleQueryInterface<bool>(
				m_wlanHandle.get(),
				m_id.Get(),
				Win32::WLAN_INTF_OPCODE::wlan_intf_opcode_background_scan_enabled
			);
		}

		auto GetChannelNumber() const -> Win32::ULONG
		{
			return SimpleQueryInterface<Win32::ULONG>(
				m_wlanHandle.get(),
				m_id.Get(),
				wlan_intf_opcode_channel_number
			);
		}

		auto IsInStreamingMediaMode() const -> bool
		{
			return SimpleQueryInterface<bool>(
				m_wlanHandle.get(),
				m_id.Get(),
				Win32::WLAN_INTF_OPCODE::wlan_intf_opcode_media_streaming_mode
			);
		}

		auto GetCurrentOperationMode() const -> Win32::ULONG
		{
			return SimpleQueryInterface<Win32::ULONG>(
				m_wlanHandle.get(),
				m_id.Get(),
				Win32::WLAN_INTF_OPCODE::wlan_intf_opcode_current_operation_mode
			);
		}

		auto IsSafeModeSupported() const -> bool
		{
			return SimpleQueryInterface<bool>(
				m_wlanHandle.get(),
				m_id.Get(),
				Win32::WLAN_INTF_OPCODE::wlan_intf_opcode_supported_safe_mode
			);
		}

		auto IsCertifiedSafeMode() const -> bool
		{
			return SimpleQueryInterface<bool>(
				m_wlanHandle.get(),
				m_id.Get(),
				Win32::WLAN_INTF_OPCODE::wlan_intf_opcode_certified_safe_mode
			);
		}

		auto GetInfrastructureCipherPairs() const -> std::vector<Win32::DOT11_AUTH_CIPHER_PAIR>
		{
			return SimpleQueryInterface<std::vector<Win32::DOT11_AUTH_CIPHER_PAIR>>(
				m_wlanHandle.get(),
				m_id.Get(),
				Win32::WLAN_INTF_OPCODE::wlan_intf_opcode_supported_adhoc_auth_cipher_pairs
			);
		}

		auto GetAdHocCipherPairs() const -> std::vector<Win32::DOT11_AUTH_CIPHER_PAIR>
		{
			return SimpleQueryInterface<std::vector<Win32::DOT11_AUTH_CIPHER_PAIR>>(
				m_wlanHandle.get(),
				m_id.Get(),
				Win32::WLAN_INTF_OPCODE::wlan_intf_opcode_supported_infrastructure_auth_cipher_pairs
			);
		}

		auto GetCapability() const -> Win32::WLAN_INTERFACE_CAPABILITY
		{
			// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/ns-wlanapi-wlan_interface_capability
			auto capability = static_cast<Win32::WLAN_INTERFACE_CAPABILITY*>(nullptr);
			// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlangetinterfacecapability
			auto status = Win32::DWORD{ 
				Win32::WlanGetInterfaceCapability(
					m_wlanHandle.get(),
					&m_id.Get(),
					nullptr,
					&capability
				) };
			if (status != Win32::ErrorCodes::Success)
				throw Error::Win32Error{status, "WlanGetInterfaceCapability() failed"};
			auto cleanup = UniqueWLANMemory{ capability };
			return *capability;
		}

	private:
		Util::GloballyUniqueID m_id;
		std::wstring m_description;
		SharedWLANHandle m_wlanHandle;
	};
}