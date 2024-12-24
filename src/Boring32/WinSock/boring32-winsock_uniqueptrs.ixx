export module boring32:winsock_uniqueptrs;
import boring32.shared;
import :raii;

export namespace Boring32::WinSock
{
	using AddrInfoWUniquePtr = RAII::UniquePtr<Win32::WinSock::ADDRINFOW, Win32::WinSock::FreeAddrInfoW>;

	void CloseSocket(Win32::WinSock::SOCKET sock)
	{
		if (sock and sock != Win32::WinSock::_INVALID_SOCKET)
			Win32::WinSock::closesocket(sock);
	}
	using SocketUniquePtr = RAII::UniquePtr<Win32::WinSock::SOCKET, CloseSocket>;
}
