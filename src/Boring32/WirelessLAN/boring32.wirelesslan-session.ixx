export module boring32.wirelesslan:session;
import :cleanup;
import :wirelessinterfaces;
import <win32.hpp>;

export namespace Boring32::WirelessLAN
{
	class Session
	{
		public:
			virtual ~Session() = default; 
			Session();
			Session(const Session& other) = default;
			Session(Session&& other) noexcept = default;

		public:
			virtual Session& operator=(const Session& other) = default;
			virtual Session& operator=(Session&& other) noexcept = default;

		public:
			virtual void Close();
			virtual DWORD GetMaxClientVersion() const noexcept;
			virtual DWORD GetNegotiatedVersion() const noexcept;
			//virtual WirelessInterfaces Interfaces();

		protected:
			virtual SharedWLANHandle Open();
			
		protected:
			SharedWLANHandle m_wlanHandle;
			DWORD m_maxClientVersion;
			DWORD m_negotiatedVersion;

			// declared here due to initialisation order
			// may need to reconsider this approach
		public:
			WirelessInterfaces Interfaces;
	};
}