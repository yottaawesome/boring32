module;

#include <string>
#include <vector>
#include <stdexcept>
#include <source_location>
#include <ostream>
#include <iostream>
#include <Windows.h>
#include <ws2tcpip.h>

module boring32.winsock:functions;
import :winsockerror;
import boring32.raii;
import boring32.error;
import boring32.async;

namespace Boring32::WinSock
{
	std::ostream& operator<<(std::ostream& os, const NetworkingAddress& addr)
	{
		return os 
			<< "Family " 
			<< std::to_string((uint32_t)addr.Family)
			<< "; Value: " 
			<< addr.Value;
	}

	std::vector<NetworkingAddress> Resolve(const std::wstring& name)
	{
		PADDRINFOW resolvedNames = nullptr;
		const int result = GetAddrInfoW(
			name.c_str(),
			L"domain",
			nullptr,
			&resolvedNames
		);
		if (result != 0)
			throw WinSockError(std::source_location::current(), "GetAddrInfoW() failed", result);

		std::vector<NetworkingAddress> names;
		for (PADDRINFOW ptr = resolvedNames; ptr != nullptr; ptr = ptr->ai_next)
		{
			switch (ptr->ai_family)
			{
				case AF_INET:
				{
					sockaddr_in* addr_in = (sockaddr_in*)ptr->ai_addr;
					char ip[INET_ADDRSTRLEN];
					inet_ntop(AF_INET, &(addr_in->sin_addr), ip, INET_ADDRSTRLEN);
					names.push_back({
						.Family = AddressFamily::IPv4,
						.Value = ip
					});
					break;
				}

				case AF_INET6:
				{
					sockaddr_in6* addr_in6 = (sockaddr_in6*)ptr->ai_addr;
					char ip[INET6_ADDRSTRLEN];
					inet_ntop(AF_INET6, &(addr_in6->sin6_addr), ip, INET6_ADDRSTRLEN);
					names.push_back({
						.Family = AddressFamily::IPv6,
						.Value = ip
					});
					break;
				}
			}
		}

		if (resolvedNames)
			FreeAddrInfoW(resolvedNames);

		return names;
	}

	struct OverlappedEx : public OVERLAPPED
	{
		OverlappedEx(Async::Event& a) : OVERLAPPED{}, e(a) {};
		Async::Event& e;
	};

	void WINAPI QueryCompleteCallback(
		DWORD Error,
		DWORD Bytes,
		LPOVERLAPPED Overlapped
	)
	{
		OverlappedEx* ex = (OverlappedEx*)Overlapped;
		ex->e.Signal();
	}

	std::vector<NetworkingAddress> Resolve2(const std::wstring& name)
	{
		HANDLE  cancelHandle = nullptr;
		PADDRINFOEXW queryResults = nullptr;
		timeval t{
			.tv_sec = 0,         /* seconds */
			.tv_usec = 10        /* and microseconds */
		};

		Async::Event e(false, true, false);
		OverlappedEx ov(e);
		ADDRINFOEX hints{ .ai_family = AF_UNSPEC };
		//https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getaddrinfoexw
		const int error = GetAddrInfoExW(
			name.c_str(),
			nullptr,
			NS_ALL,
			nullptr,
			&hints,
			&queryResults,
			nullptr,//&t,
			&ov,
			QueryCompleteCallback,
			&cancelHandle
		);
		if (error != WSA_IO_PENDING)
			throw WinSockError(std::source_location::current(), "GetAddrInfoExW() failed", error);
		e.WaitOnEvent();

		if (ov.InternalHigh != NOERROR)
			throw Error::Win32Error(std::source_location::current(), "GetAddrInfoExW() overlapped op failed", (DWORD)ov.InternalHigh);

		std::vector<NetworkingAddress> names;
		for (PADDRINFOEXW ptr = queryResults; ptr != nullptr; ptr = ptr->ai_next)
		{
			switch (ptr->ai_family)
			{
				case AF_INET:
				{
					std::string ip(INET_ADDRSTRLEN, '\0');
					sockaddr_in* addr_in = (sockaddr_in*)ptr->ai_addr;
					inet_ntop(AF_INET, &(addr_in->sin_addr), &ip[0], ip.length());
					names.push_back({
						.Family = AddressFamily::IPv4,
						.Value = std::move(ip)
					});
					break;
				}

				case AF_INET6:
				{
					std::string ip(INET6_ADDRSTRLEN, '\0');
					sockaddr_in6* addr_in6 = (sockaddr_in6*)ptr->ai_addr;
					inet_ntop(AF_INET6, &(addr_in6->sin6_addr), &ip[0], ip.length());
					names.push_back({
						.Family = AddressFamily::IPv6,
						.Value = std::move(ip)
					});
					break;
				}
			}
		}

		if (queryResults)
			FreeAddrInfoExW(queryResults);

		return names;
	}

	std::vector<NetworkingAddress> Resolve3(const std::wstring& name)
	{
		Async::Event opCompleted(false, true, false);
		OVERLAPPED ov{ .hEvent = opCompleted.GetHandle() };
		PADDRINFOEXW queryResults = nullptr;
		timeval timeout{
			.tv_sec = 10,         /* seconds */
			.tv_usec = 0        /* and microseconds */
		};
		ADDRINFOEX hints{.ai_family = AF_UNSPEC};
		const int error = GetAddrInfoExW(
			name.c_str(),
			nullptr,
			NS_DNS,
			nullptr,
			&hints,
			&queryResults,
			&timeout,
			&ov,
			nullptr,
			nullptr
		);
		if (error != WSA_IO_PENDING)
			throw WinSockError(std::source_location::current(), "GetAddrInfoExW() failed", error);

		opCompleted.WaitOnEvent();
		if (ov.InternalHigh != NOERROR)
			throw Error::Win32Error(std::source_location::current(), "GetAddrInfoExW() overlapped op failed", (DWORD)ov.InternalHigh);

		std::vector<NetworkingAddress> names;
		for (PADDRINFOEXW ptr = queryResults; ptr != nullptr; ptr = ptr->ai_next)
		{
			switch (ptr->ai_family)
			{
				case AF_INET:
				{
					std::string ip(INET_ADDRSTRLEN, '\0');
					sockaddr_in* addr_in = (sockaddr_in*)ptr->ai_addr;
					inet_ntop(AF_INET, &(addr_in->sin_addr), &ip[0], ip.length());
					names.push_back({
						.Family = AddressFamily::IPv4,
						.Value = std::move(ip)
					});
					break;
				}

				case AF_INET6:
				{
					std::string ip(INET6_ADDRSTRLEN, '\0');
					sockaddr_in6* addr_in6 = (sockaddr_in6*)ptr->ai_addr;
					inet_ntop(AF_INET6, &(addr_in6->sin6_addr), &ip[0], ip.length());
					names.push_back({
						.Family = AddressFamily::IPv6,
						.Value = std::move(ip)
					});
					break;
				}
			}
		}

		if (queryResults)
			FreeAddrInfoExW(queryResults);

		return names;
	}
}
