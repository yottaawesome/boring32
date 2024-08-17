export module boring32:winsock_uniqueptrs;
import boring32.shared;

export namespace Boring32::WinSock
{
	struct AddrInfoWDeleter final
	{
		void operator()(Win32::WinSock::ADDRINFOW* obj)
		{
			Win32::WinSock::FreeAddrInfoW(obj);
		}
	};
	using AddrInfoWUniquePtr = std::unique_ptr<Win32::WinSock::ADDRINFOW, AddrInfoWDeleter>;

	struct SocketDeleter final
	{
		void operator()(Win32::WinSock::SOCKET sock)
		{
			if (sock and sock != Win32::WinSock::_INVALID_SOCKET)
				Win32::WinSock::closesocket(sock);
		}
	};
	using SocketUniquePtr = std::unique_ptr<Win32::WinSock::SOCKET, SocketDeleter>;
}
