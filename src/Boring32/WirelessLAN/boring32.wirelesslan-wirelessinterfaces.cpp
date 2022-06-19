module boring32.wirelesslan:wirelessinterfaces;
import :cleanup;

namespace Boring32::WirelessLAN
{
	WirelessInterfaces::~WirelessInterfaces() { }

	WirelessInterfaces::WirelessInterfaces(SharedWLANHandle session)
		: m_session(session)
	{
	}
}