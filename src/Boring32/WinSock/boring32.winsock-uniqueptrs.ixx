export module boring32.winsock:uniqueptrs;
import std;
import std.compat;
import boring32.win32;

export namespace Boring32::WinSock
{
	struct AddrInfoWDeleter final
	{
		void operator()(Win32::WinSock::ADDRINFOW* obj)
		{
			Win32::WinSock::FreeAddrInfoW(obj);
		}
	};
	using AddrInfoWUniquePtr = std::unique_ptr<Win32::WinSock::ADDRINFOW, AddrInfoWDeleter>;
}
