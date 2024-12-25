export module boring32:winsock_winsockerror;
import std;
import boring32.win32;
import :error;

export namespace Boring32::WinSock
{
	struct WinSockError : Error::Boring32Error
	{
		virtual ~WinSockError() = default;
		WinSockError(
			const std::string& message,
			const std::source_location& location = std::source_location::current(),
			const std::stacktrace trace = std::stacktrace::current()
		) : Error::Boring32Error(message, location, trace)
		{ }
	};
}