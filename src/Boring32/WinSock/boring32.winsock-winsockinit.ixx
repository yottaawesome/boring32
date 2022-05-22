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
			WinSockInit(const DWORD highVersion, const DWORD lowVersion);

		public:
			const WSAData& GetData() const noexcept;

		protected:
			virtual void Initialize(const DWORD highVersion, const DWORD lowVersion);

		protected:
			WSAData m_wsaData;
	};
}