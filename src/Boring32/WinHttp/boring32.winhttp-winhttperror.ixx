module;

#include <source_location>

export module boring32.winhttp:winhttperror;
import boring32.error;
import <string>;
import <win32.hpp>;

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