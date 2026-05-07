export module boring32:winsock.functions;
import std;
import :win32;
import :raii;
import :error;
import :strings;
import :async;
import :winsock.winsockerror;

namespace Boring32::WinSock
{
	template<Async::AnyEvent T>
	struct OverlappedEx : Win32::OVERLAPPED
	{
		OverlappedEx(T& a) : Win32::OVERLAPPED{}, e(a) {};
		T& e;
	};

	template<typename TAnyEvent>
	void QueryCompleteCallback(
		Win32::DWORD Error,
		Win32::DWORD Bytes,
		Win32::LPOVERLAPPED Overlapped
	)
	{
		auto* ex = reinterpret_cast<OverlappedEx<TAnyEvent>*>(Overlapped);
		ex->e.Signal();
	}
}

export namespace Boring32::WinSock
{
	void IPv4NetworkAddressToString(unsigned int ip, std::string& out)
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-htonl
		auto converted = Win32::ULONG{ Win32::WinSock::htonl(ip) };

		out.resize(Win32::WinSock::_INET6_ADDRSTRLEN);
		// https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-inet_ntop
		auto ipCString = Win32::PCSTR{Win32::WinSock::inet_ntop(Win32::WinSock::AddressFamily::IPv6, &converted, &out[0], Win32::WinSock::_INET6_ADDRSTRLEN)};
		if (not ipCString)
		{
			Error::ThrowNested(
				Error::Win32Error{static_cast<Win32::DWORD>(Win32::WinSock::WSAGetLastError()), "inet_ntop() failed", L"Ws2_32.dll"},
				WinSockError("Could not convert IPv6 network address string")
			);
		}
		out = out.c_str();
	}

	// Adapted from https://stackoverflow.com/a/54314540
	void IPv6NetworkAddressToString(const unsigned int ip, std::string& out)
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-htonl
		auto converted = Win32::ULONG{ Win32::WinSock::htonl(ip) };

		out.resize(Win32::WinSock::_INET_ADDRSTRLEN);
		// https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-inet_ntop
		auto ipCString = Win32::PCSTR{Win32::WinSock::inet_ntop(Win32::WinSock::AddressFamily::IPv4, &converted, &out[0], Win32::WinSock::_INET_ADDRSTRLEN)};
		if (not ipCString)
		{
			Error::ThrowNested(
				Error::Win32Error{static_cast<Win32::DWORD>(Win32::WinSock::WSAGetLastError()), "inet_ntop() failed", L"Ws2_32.dll"},
				WinSockError("Could not convert IPv4 network address string")
			);
		}
		out = out.c_str();
	}

	void IPv4NetworkAddressToString(unsigned int ip, std::wstring& out)
	{
		auto str = std::string{};
		IPv6NetworkAddressToString(ip, str);
		out = Boring32::Strings::ConvertString(str);
	}

	void IPv6NetworkAddressToString(unsigned int ip, std::wstring& out)
	{
		auto str = std::string{};
		IPv4NetworkAddressToString(ip, str);
		out = Boring32::Strings::ConvertString(str);
	}

	enum class AddressFamily : std::uint32_t
	{
		Unknown,
		IPv4 = 1,
		IPv6
	};

	struct NetworkingAddress
	{
		AddressFamily Family = AddressFamily::Unknown;
		std::string Value;
	};

	auto Resolve(const std::wstring& name) -> std::vector<NetworkingAddress>
	{
		auto resolvedNames = Win32::WinSock::PADDRINFOW{};
		auto result = Win32::WinSock::GetAddrInfoW(
			name.c_str(),
			L"domain",
			nullptr,
			&resolvedNames
		);
		if (result != 0)
		{
			Error::ThrowNested(
				Error::Win32Error{static_cast<Win32::DWORD>(result), "GetAddrInfoW() failed", L"Ws2_32.dll"},
				WinSockError("Could not get domain addr info")
			);
		}

		auto names = std::vector<NetworkingAddress>{};
		for (Win32::WinSock::PADDRINFOW ptr = resolvedNames; ptr != nullptr; ptr = ptr->ai_next)
		{
			switch (ptr->ai_family)
			{
				case Win32::WinSock::AddressFamily::IPv4:
				{
					auto addr_in = reinterpret_cast<Win32::WinSock::sockaddr_in*>(ptr->ai_addr);
					auto ip = std::array<char, Win32::WinSock::_INET_ADDRSTRLEN>{};
					Win32::WinSock::inet_ntop(Win32::WinSock::AddressFamily::IPv4, &(addr_in->sin_addr), ip.data(), ip.size());
					names.push_back({
						.Family = AddressFamily::IPv4,
						.Value = ip.data()
					});
					break;
				}

				case Win32::WinSock::AddressFamily::IPv6:
				{
					auto addr_in6 = reinterpret_cast<Win32::WinSock::sockaddr_in6*>(ptr->ai_addr);
					auto ip = std::array<char, Win32::WinSock::_INET6_ADDRSTRLEN>{};
					Win32::WinSock::inet_ntop(Win32::WinSock::AddressFamily::IPv6, &(addr_in6->sin6_addr), ip.data(), ip.size());
					names.push_back({
						.Family = AddressFamily::IPv6,
						.Value = ip.data()
					});
					break;
				}
			}
		}

		if (resolvedNames)
			Win32::WinSock::FreeAddrInfoW(resolvedNames);

		return names;
	}

	auto Resolve2(const std::wstring& name) -> std::vector<NetworkingAddress>
	{
		auto cancelHandle = Win32::HANDLE{};
		auto queryResults = Win32::WinSock::PADDRINFOEXW{};
		auto t = Win32::timeval{
			.tv_sec = 0,         /* seconds */
			.tv_usec = 10        /* and microseconds */
		};

		auto e = Async::ManualResetEvent{ false, false };
		auto ov = Win32::OVERLAPPED{ .hEvent = e.GetHandle() };
		auto hints = Win32::WinSock::ADDRINFOEX{ .ai_family = Win32::WinSock::AddressFamily::IPv4 };
		//https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getaddrinfoexw
		auto error = Win32::WinSock::GetAddrInfoExW(
			name.c_str(),
			nullptr,
			Win32::_NS_ALL,
			nullptr,
			&hints,
			&queryResults,
			nullptr,//&t,
			&ov,
			QueryCompleteCallback<decltype(e)>,
			&cancelHandle
		);
		if (error != Win32::WinSock::_WSA_IO_PENDING) Error::ThrowNested(
			Error::Win32Error{static_cast<Win32::DWORD>(error), "GetAddrInfoExW() failed", L"Ws2_32.dll"},
			WinSockError("Could not get domain addr info")
		);
		e.WaitOnEvent();

		if (ov.InternalHigh != Win32::ErrorCodes::NoError) Error::ThrowNested(
			Error::Win32Error{(Win32::DWORD)ov.InternalHigh, "GetAddrInfoExW() overlapped op failed"},
			WinSockError("Could not get domain addr info")
		);

		auto names = std::vector<NetworkingAddress>{};
		for (Win32::WinSock::PADDRINFOEXW ptr = queryResults; ptr != nullptr; ptr = ptr->ai_next)
		{
			switch (ptr->ai_family)
			{
				case Win32::WinSock::AddressFamily::IPv4:
				{
					auto ip = std::string(Win32::WinSock::_INET_ADDRSTRLEN, '\0');
					auto addr_in = reinterpret_cast<Win32::WinSock::sockaddr_in*>(ptr->ai_addr);
					Win32::WinSock::inet_ntop(Win32::WinSock::AddressFamily::IPv4, &(addr_in->sin_addr), &ip[0], ip.length());
					names.push_back({
						.Family = AddressFamily::IPv4,
						.Value = std::move(ip)
					});
					break;
				}

				case Win32::WinSock::AddressFamily::IPv6:
				{
					auto ip = std::string(Win32::WinSock::_INET6_ADDRSTRLEN, '\0');
					auto addr_in6 = reinterpret_cast<Win32::WinSock::sockaddr_in6*>(ptr->ai_addr);
					Win32::WinSock::inet_ntop(Win32::WinSock::AddressFamily::IPv6, &(addr_in6->sin6_addr), &ip[0], ip.length());
					names.push_back({
						.Family = AddressFamily::IPv6,
						.Value = std::move(ip)
					});
					break;
				}
			}
		}

		if (queryResults)
			Win32::WinSock::FreeAddrInfoExW(queryResults);

		return names;
	}

	auto Resolve3(const std::wstring& name) -> std::vector<NetworkingAddress>
	{
		auto opCompleted = Async::ManualResetEvent{ false, false };
		auto ov = Win32::OVERLAPPED{ .hEvent = opCompleted.GetHandle() };
		auto queryResults = Win32::WinSock::PADDRINFOEXW{};
		auto timeout = Win32::timeval{
			.tv_sec = 10,         /* seconds */
			.tv_usec = 0        /* and microseconds */
		};
		auto hints = Win32::WinSock::ADDRINFOEX{ .ai_family = Win32::WinSock::AddressFamily::Unspecified };
		auto error = Win32::WinSock::GetAddrInfoExW(
			name.c_str(),
			nullptr,
			Win32::_NS_DNS,
			nullptr,
			&hints,
			&queryResults,
			&timeout,
			&ov,
			nullptr,
			nullptr
		);
		if (error != Win32::WinSock::_WSA_IO_PENDING) Error::ThrowNested(
			Error::Win32Error{static_cast<Win32::DWORD>(error), "GetAddrInfoExW() failed", L"Ws2_32.dll"},
			WinSockError("Could not get domain addr info")
		);

		opCompleted.WaitOnEvent();
		if (ov.InternalHigh != Win32::ErrorCodes::NoError) Error::ThrowNested(
			Error::Win32Error{(Win32::DWORD)ov.InternalHigh, "GetAddrInfoExW() overlapped op failed"},
			WinSockError("Could not get domain addr info")
		);

		auto names = std::vector<NetworkingAddress>{};
		for (Win32::WinSock::PADDRINFOEXW ptr = queryResults; ptr != nullptr; ptr = ptr->ai_next)
		{
			switch (ptr->ai_family)
			{
				case Win32::WinSock::AddressFamily::IPv4:
				{
					auto ip = std::string(Win32::WinSock::_INET_ADDRSTRLEN, '\0');
					auto addr_in = reinterpret_cast<Win32::WinSock::sockaddr_in*>(ptr->ai_addr);
					Win32::WinSock::inet_ntop(Win32::WinSock::AddressFamily::IPv4, &(addr_in->sin_addr), &ip[0], ip.length());
					names.push_back({
						.Family = AddressFamily::IPv4,
						.Value = std::move(ip)
					});
					break;
				}

				case Win32::WinSock::AddressFamily::IPv6:
				{
					auto ip = std::string(Win32::WinSock::_INET6_ADDRSTRLEN, '\0');
					auto addr_in6 = reinterpret_cast<Win32::WinSock::sockaddr_in6*>(ptr->ai_addr);
					Win32::WinSock::inet_ntop(Win32::WinSock::AddressFamily::IPv6, &(addr_in6->sin6_addr), &ip[0], ip.length());
					names.push_back({
						.Family = AddressFamily::IPv6,
						.Value = std::move(ip)
					});
					break;
				}
			}
		}

		if (queryResults)
			Win32::WinSock::FreeAddrInfoExW(queryResults);

		return names;
	}

	auto operator<<(std::ostream& os, const NetworkingAddress& addr) -> std::ostream&
	{
		return os
			<< "Family "
			<< std::to_string((uint32_t)addr.Family)
			<< "; Value: "
			<< addr.Value;
	}
}
