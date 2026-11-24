export module boring32:winsock.winsockerror;
import std;
import :win32;
import :error;

export namespace Boring32::WinSock
{
	struct WinSockError final : public Error::Win32Error
	{
		WinSockError(
			const std::string& message,
			const std::source_location& location = std::source_location::current(),
			const std::stacktrace trace = std::stacktrace::current()
		) : Error::Win32Error{ message, location, trace }
		{ }

		WinSockError(
			int errorCode,
			const std::string& message,
			const std::source_location& location = std::source_location::current(),
			const std::stacktrace trace = std::stacktrace::current()
		) : Error::Win32Error{static_cast<Win32::DWORD>(errorCode), message, L"ws2_32.dll", location, trace}
		{ }
	};
}