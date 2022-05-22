module;

#include <windows.h>
#include <winsock2.h>

export module boring32.winsock:winsockinit;

export namespace Boring32::WinSock
{
	class WinSockInit
	{
		public:
			virtual ~WinSockInit();
			WinSockInit();
			WinSockInit(const WinSockInit& other);
			WinSockInit(const DWORD highVersion, const DWORD lowVersion);

		public:
			const WSAData& GetData() const noexcept;
			virtual void Close();

		protected:
			virtual void Initialize();
			virtual WinSockInit& Copy(const WinSockInit& other);

		protected:
			WSAData m_wsaData;
			DWORD m_lowVersion;
			DWORD m_highVersion;
	};
}