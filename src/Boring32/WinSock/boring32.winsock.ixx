module;

#include "pch.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

export module boring32.winsock;

export namespace Boring32::WinSock
{
	class WinSockInit
	{
		public:
			WinSockInit();
			WinSockInit(const DWORD highVersion, const DWORD lowVersion);
			virtual ~WinSockInit();

		public:
			const WSAData& GetData() const noexcept;

		protected:
			virtual void Initialize(const DWORD highVersion, const DWORD lowVersion);

		protected:
			WSAData m_wsaData;
	};

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
