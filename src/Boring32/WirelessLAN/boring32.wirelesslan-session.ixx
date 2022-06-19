module;

#include <Windows.h>

export module boring32.wirelesslan:session;
import :cleanup;

export namespace Boring32::WirelessLAN
{
	class Session
	{
		public:
			virtual ~Session(); 
			Session();

		public:
			virtual DWORD GetMaxClientVersion() const noexcept;
			virtual DWORD GetNegotiatedVersion() const noexcept;

		protected:
			virtual void Open();
			
		protected:
			SharedWLANHandle m_wlanHandle;
			DWORD m_maxClientVersion;
			DWORD m_negotiatedVersion;
	};
}