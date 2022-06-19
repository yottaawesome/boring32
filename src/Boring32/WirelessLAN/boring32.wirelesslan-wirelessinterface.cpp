module;

#include <string>
#include <Windows.h>
#include <wlanapi.h>

module boring32.wirelesslan:wirelessinterface;
import boring32.util;

namespace Boring32::WirelessLAN
{
	WirelessInterface::~WirelessInterface()
	{
	}

	WirelessInterface::WirelessInterface(
		const Util::GloballyUniqueID& id,
		std::wstring description
	) : m_id(id), 
		m_description(std::move(m_description))
	{

	}

	const Util::GloballyUniqueID& WirelessInterface::GUID() const noexcept
	{
		return m_id;
	}

	const std::wstring& WirelessInterface::Description() const noexcept
	{
		return m_description;
	}
}