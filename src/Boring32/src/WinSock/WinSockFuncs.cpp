module;

#include <string>
#include <vector>
#include <stdexcept>
#include <ws2tcpip.h>
#include <ostream>

module boring32.winsock;

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
			&name[0],
			L"domain",
			nullptr,
			&resolvedNames
		);
		if (result != 0)
		{
			throw WinSockError(__FUNCSIG__ ": GetAddrInfoW() failed", result);
		}

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

		FreeAddrInfoW(resolvedNames);

		return names;
	}
}