module;

#include <stdexcept>
#include <windows.h>
#include <winsock2.h>

export module boring32.winsock:winsockinit;

export namespace Boring32::WinSock
{
	class WinSockInit
	{
		// Move semantics don't make sense for this object.
		public:
			virtual ~WinSockInit();
			WinSockInit();
			WinSockInit(const WinSockInit& other);
			WinSockInit(WinSockInit&&) noexcept = delete;

			WinSockInit(const DWORD highVersion, const DWORD lowVersion);

		public:
			virtual WinSockInit& operator=(const WinSockInit& other);
			virtual WinSockInit& operator=(WinSockInit&&) noexcept = delete;

		public:
			virtual void Close();
			virtual bool Close(const std::nothrow_t&);
			virtual DWORD GetLowVersion() const noexcept;
			virtual DWORD GetHighVersion() const noexcept;

		protected:
			virtual void Initialize();
			virtual WinSockInit& Copy(const WinSockInit& other);

		protected:
			WSAData m_wsaData;
			DWORD m_lowVersion;
			DWORD m_highVersion;
	};
}