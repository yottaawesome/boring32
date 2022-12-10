module;

#include <source_location>

module boring32.winsock:winsockerror;

namespace Boring32::WinSock
{
	WinSockError::~WinSockError() { }

	WinSockError::WinSockError(
		const std::string& message,
		const std::source_location location
	) : Error::Boring32Error(message, location)
	{
	}
}