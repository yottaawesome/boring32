export module boring32.winsock:resolvedname;
import std;
import boring32.win32;
import boring32.error;
import :uniqueptrs;
import :winsockerror;

export namespace Boring32::WinSock
{
	struct ResolvedName
	{
		std::wstring Name{};
		unsigned short Port = 80;
		Win32::WinSock::AddressFamily::Family Type = Win32::WinSock::AddressFamily::Unspecified;
		Win32::WinSock::IPPROTO Protocol = Win32::WinSock::IPPROTO::IPPROTO_TCP;

		AddrInfoWUniquePtr AddrInfo = 
			[](
				std::wstring_view name, 
				unsigned short port, 
				Win32::WinSock::AddressFamily::Family type, 
				Win32::WinSock::IPPROTO protocol
			)
			{
				Win32::WinSock::ADDRINFOW hints{
					.ai_family = type,
					//.ai_socktype = Win32::WinSock::_SOCK_STREAM,
					.ai_protocol = protocol
				};

				Win32::WinSock::ADDRINFOW* addrInfoResult;
				std::wstring portNumber = port ? std::to_wstring(port) : L"";
				// https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getaddrinfow
				const int status = Win32::WinSock::GetAddrInfoW(
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
			}(Name, Port, Type, Protocol);
	};
}