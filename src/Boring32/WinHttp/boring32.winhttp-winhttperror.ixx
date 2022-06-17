module;

#include <string>
#include <source_location>
#include <Windows.h>

export module boring32.winhttp:winhttperror;
import boring32.error;

export namespace Boring32::WinHttp
{
	class WinHttpError : public Error::Boring32Error
	{
		public:
			virtual ~WinHttpError();
			WinHttpError(
				const std::string& msg,
				const std::source_location location = std::source_location::current()
			);
	};
}