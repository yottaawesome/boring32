module;

#include "pch.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

export module boring32.winsock;

export namespace Boring32::WinSock
{
	class WinSockError : public std::runtime_error
	{
		public:
			virtual ~WinSockError();
			WinSockError(const DWORD errorCode);
			WinSockError(const std::string& message, const DWORD errorCode);

			virtual DWORD GetErrorCode() const noexcept;
			virtual const char* what() const noexcept override;

		protected:
			DWORD m_errorCode;
			std::string m_errorString;
	};

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

	std::ostream& operator<<(std::ostream& os, const NetworkingAddress& addr);
}
