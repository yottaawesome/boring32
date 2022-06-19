module;

#include <Windows.h>

export module boring32.wirelesslan:session;
import :cleanup;
import :wirelessinterfaces;

export namespace Boring32::WirelessLAN
{
	class Session
	{
		public:
			virtual ~Session(); 
			Session();
			Session(const Session& other);
			Session(Session&& other) noexcept;

		public:
			virtual Session& operator=(const Session& other);
			virtual Session& operator=(Session&& other) noexcept;

		public:
			virtual void Close();
			virtual DWORD GetMaxClientVersion() const noexcept;
			virtual DWORD GetNegotiatedVersion() const noexcept;
			virtual WirelessInterfaces Interfaces();

		protected:
			virtual void Open();
			
		protected:
			SharedWLANHandle m_wlanHandle;
			DWORD m_maxClientVersion;
			DWORD m_negotiatedVersion;
	};
}