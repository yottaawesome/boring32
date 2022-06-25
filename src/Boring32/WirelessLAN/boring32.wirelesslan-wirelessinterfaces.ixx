module;

#include <vector>

export module boring32.wirelesslan:wirelessinterfaces;
import :cleanup;
import :wirelessinterface;

export namespace Boring32::WirelessLAN
{
	class WirelessInterfaces
	{
		public:
			virtual ~WirelessInterfaces();
			WirelessInterfaces(SharedWLANHandle session);

		public:
			virtual std::vector<WirelessInterface> GetAll() const;

		protected:
			SharedWLANHandle m_session;
	};
}