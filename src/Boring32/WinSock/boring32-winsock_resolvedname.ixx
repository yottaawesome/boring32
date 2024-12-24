export module boring32:winsock_resolvedname;
import boring32.shared;
import :error;
import :winsock_uniqueptrs;
import :winsock_winsockerror;

export namespace Boring32::WinSock
{
	struct ResolvedName
	{
		std::wstring Name{};
		unsigned Port = 80;
		Win32::WinSock::AddressFamily::Family AddressType = Win32::WinSock::AddressFamily::Unspecified;
		Win32::WinSock::IPPROTO Protocol = Win32::WinSock::IPPROTO::IPPROTO_TCP;
		int SocketType = Win32::WinSock::_SOCK_STREAM;
		AddrInfoWUniquePtr AddrInfo = ResolvedName::Resolve(Name, Port, AddressType, Protocol, SocketType);

		static AddrInfoWUniquePtr Resolve(
			std::wstring_view name,
			unsigned port,
			Win32::WinSock::AddressFamily::Family addressType,
			Win32::WinSock::IPPROTO protocol,
			int socketType
		)
		{
			if (name.empty())
				return AddrInfoWUniquePtr{};

			Win32::WinSock::ADDRINFOW hints{
				.ai_family = addressType,
				.ai_socktype = socketType,
				.ai_protocol = protocol
			};

			Win32::WinSock::ADDRINFOW* addrInfoResult;
			std::wstring portNumber = port ? std::to_wstring(port) : L"";
			// https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getaddrinfow
			int status = Win32::WinSock::GetAddrInfoW(
				name.data(),
				portNumber.c_str(),
				&hints,
				&addrInfoResult
			);
			if (status)
				throw Error::Win32Error("GetAddrInfoW() failed", status, L"ws2_32.dll");
			if (not addrInfoResult)
				throw WinSockError("GetAddrInfoW() did not find any valid results");

			return AddrInfoWUniquePtr(addrInfoResult);
		}
	};
}