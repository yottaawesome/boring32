export module boring32.winsock:resolvedname;
import boring32.win32;
import :uniqueptrs;

export namespace Boring32::WinSock
{
	class ResolvedName
	{
		std::wstring Name{};
		AddrInfoWUniquePtr m_addrInfo;
	};
}