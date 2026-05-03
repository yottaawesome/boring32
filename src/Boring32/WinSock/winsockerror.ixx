export module boring32:winsock.winsockerror;
import std;
import :win32;
import :error;

export namespace Boring32::WinSock
{
	struct WinSockError final : Error::Boring32Error
	{
		WinSockError(
			const std::string& message,
			const std::source_location& location = std::source_location::current(),
			const std::stacktrace trace = std::stacktrace::current()
		) : Error::Boring32Error(message, location, trace)
		{ }
	};
}