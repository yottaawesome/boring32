export module Boring32:WinSock.UniquePtrs;
import std;
import :Win32;
import :Raii;

export namespace Boring32::WinSock
{
	using AddrInfoWUniquePtr = RAII::UniquePtr<Win32::WinSock::ADDRINFOW, Win32::WinSock::FreeAddrInfoW>;

	void CloseSocket(Win32::WinSock::SOCKET sock)
	{
		if (sock and sock != Win32::WinSock::InvalidSocket)
			Win32::WinSock::closesocket(sock);
	}
	using SocketUniquePtr = RAII::UniquePtr<Win32::WinSock::SOCKET, CloseSocket>;
}
