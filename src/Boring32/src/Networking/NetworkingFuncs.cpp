module;

#include <string>
#include <stdexcept>
#include <ws2tcpip.h>

module boring32.winsock;

namespace Boring32::WinSock
{
	ResolvedNames Resolve(const std::wstring& name)
	{
		PADDRINFOW resolvedNames;
		int result = GetAddrInfoW(
			&name[0],
			L"domain",
			nullptr,
			&resolvedNames
		);
		if (result != 0)
		{
			throw std::runtime_error(__FUNCSIG__ ": failed with " + result);
		}

		for (PADDRINFOW ptr = resolvedNames; ptr != nullptr; ptr = ptr->ai_next)
		{
			
		}

		FreeAddrInfoW(resolvedNames);

		return {};
	}
}