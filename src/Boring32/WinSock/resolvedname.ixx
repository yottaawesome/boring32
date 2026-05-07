export module boring32:winsock.resolvedname;
import std;
import :win32;
import :error;
import :winsock.uniqueptrs;
import :winsock.winsockerror;

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

		//ResolvedName(const ResolvedName& other) = default;
		//auto operator=(const ResolvedName& other) -> ResolvedName & = default;

		static auto Resolve(
			std::wstring_view name,
			unsigned port,
			Win32::WinSock::AddressFamily::Family addressType,
			Win32::WinSock::IPPROTO protocol,
			int socketType
		) -> AddrInfoWUniquePtr
		{
			if (name.empty())
				return AddrInfoWUniquePtr{};

			auto hints = Win32::WinSock::ADDRINFOW{
				.ai_family = addressType,
				.ai_socktype = socketType,
				.ai_protocol = protocol
			};

			auto addrInfoResult = static_cast<Win32::WinSock::ADDRINFOW*>(nullptr);
			auto portNumber = port ? std::to_wstring(port) : std::wstring{};
			// https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getaddrinfow
			auto status = Win32::WinSock::GetAddrInfoW(
				name.data(),
				portNumber.c_str(),
				&hints,
				&addrInfoResult
			);
			if (status)
				throw Error::Win32Error{static_cast<Win32::DWORD>(status), "GetAddrInfoW() failed", L"ws2_32.dll"};
			if (not addrInfoResult)
				throw WinSockError("GetAddrInfoW() did not find any valid results");

			return AddrInfoWUniquePtr(addrInfoResult);
		}
	};
}