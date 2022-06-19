module;

#include <string>
#include <Windows.h>
#include <wlanapi.h>

module boring32.wirelesslan:interfaces;
import boring32.util;

namespace Boring32::WirelessLAN
{
	Interface::~Interface()
	{
	}

	Interface::Interface(
		const Util::GloballyUniqueID& id,
		std::wstring description
	) : m_id(id), 
		m_description(std::move(m_description))
	{

	}

	const Util::GloballyUniqueID& Interface::GUID() const noexcept
	{
		return m_id;
	}

	const std::wstring& Interface::Description() const noexcept
	{
		return m_description;
	}
}