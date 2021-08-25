module;

#include "pch.hpp"
#include <vector>
#include <string>

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

	struct ResolvedNames
	{
		std::vector<std::wstring> Names;
	};

	ResolvedNames Resolve(const std::wstring& name);
}
