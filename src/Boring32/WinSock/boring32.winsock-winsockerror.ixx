module;

#include <string>
#include <source_location>
#include <stdexcept>
#include <Windows.h>

export module boring32.winsock:winsockerror;
import boring32.error;

export namespace Boring32::WinSock
{
	class WinSockError : public Error::Boring32Error
	{
		public:
			virtual ~WinSockError();
			WinSockError(
				const std::string& message,
				const std::source_location location = std::source_location::current()
			);
	};
}