export module boring32.wirelesslan:wirelessinterfaces;
import :cleanup;

export namespace Boring32::WirelessLAN
{
	class WirelessInterfaces
	{
		public:
			virtual ~WirelessInterfaces();
			WirelessInterfaces(SharedWLANHandle session);

		protected:
			SharedWLANHandle m_session;
	};
}