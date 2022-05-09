module;

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

export module boring32.winsock:functions;

export namespace Boring32::WinSock
{
	void IPv4NetworkAddressToString(const unsigned int ip, std::string& out);
	void IPv4NetworkAddressToString(const unsigned int ip, std::wstring& out);
	void IPv6NetworkAddressToString(const unsigned int ip, std::string& out);
	void IPv6NetworkAddressToString(const unsigned int ip, std::wstring& out);
	enum class AddressFamily : uint32_t
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

	std::vector<NetworkingAddress> Resolve(const std::wstring& name);
	std::vector<NetworkingAddress> Resolve2(const std::wstring& name);
	std::vector<NetworkingAddress> Resolve3(const std::wstring& name);

	std::ostream& operator<<(std::ostream& os, const NetworkingAddress& addr);
}
