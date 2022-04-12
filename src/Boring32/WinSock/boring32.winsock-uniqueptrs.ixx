module;

#include <memory>
#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

export module boring32.winsock:uniqueptrs;

export namespace Boring32::WinSock
{
	struct AddrInfoWDeleter final
	{
		void operator()(ADDRINFOW* obj)
		{
			FreeAddrInfoW(obj);
		}
	};
	using AddrInfoWUniquePtr = std::unique_ptr<ADDRINFOW, AddrInfoWDeleter>;
}
