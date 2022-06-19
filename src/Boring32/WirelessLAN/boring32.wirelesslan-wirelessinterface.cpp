module;

#include <string>
#include <source_location>
#include <Windows.h>
#include <wlanapi.h>

module boring32.wirelesslan:wirelessinterface;
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
		// This will point to memory allocated by WLAN; we're responsible for freeing it.
		PVOID pWlanAllocatedMemory;
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

	WirelessInterface::~WirelessInterface()
	{
	}

	WirelessInterface::WirelessInterface(
		const SharedWLANHandle& wlanHandle,
		const Util::GloballyUniqueID& id,
		std::wstring description
	) : m_wlanHandle(wlanHandle),
		m_id(id),
		m_description(std::move(m_description))
	{

	}

	const Util::GloballyUniqueID& WirelessInterface::GetGUID() const noexcept
	{
		return m_id;
	}

	const std::wstring& WirelessInterface::GetDescription() const noexcept
	{
		return m_description;
	}

	InterfaceState WirelessInterface::GetState() const
	{
		return SimpleQueryInterface<InterfaceState>(
			m_wlanHandle.get(),
			m_id.Get(),
			wlan_intf_opcode_interface_state
		);
	}

	WLAN_CONNECTION_ATTRIBUTES WirelessInterface::GetAttributes() const
	{
		return SimpleQueryInterface<WLAN_CONNECTION_ATTRIBUTES>(
			m_wlanHandle.get(), 
			m_id.Get(),
			wlan_intf_opcode_current_connection
		);
	}
}