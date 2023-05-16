module;

#include <source_location>;

export module boring32.winsock:winsockerror;
import boring32.error;
import <string>;
import <stdexcept>;
import <source_location>;
import <stacktrace>;
import <win32.hpp>;

export namespace Boring32::WinSock
{
	class WinSockError : public Error::Boring32Error
	{
		public:
			virtual ~WinSockError() = default;
			WinSockError(
				const std::string& message,
				const std::source_location& location = std::source_location::current(),
				const std::stacktrace trace = std::stacktrace::current()
			) : Error::Boring32Error(message, location, trace)
			{
			}
	};
}