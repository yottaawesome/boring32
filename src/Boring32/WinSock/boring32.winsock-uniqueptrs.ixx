export module boring32.winsock:uniqueptrs;
import std;

import <win32.hpp>;

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
