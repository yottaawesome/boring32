module;

#include <string>
#include <source_location>
#include <Windows.h>

module boring32.winsock:winsockerror;

namespace Boring32::WinSock
{
	WinSockError::~WinSockError() { }

	WinSockError::WinSockError(
		const std::source_location& location,
		const std::string& message
	) : Error::Boring32Error(location, message)
	{
	}
}