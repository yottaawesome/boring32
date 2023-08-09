export module boring32.winhttp:winhttperror;
import boring32.error;
import std;
import <win32.hpp>;


export namespace Boring32::WinHttp
{
	class WinHttpError : public Error::Boring32Error
	{
		public:
			virtual ~WinHttpError() = default;
			WinHttpError(
				const std::string& msg,
				const std::source_location& location = std::source_location::current(),
				const std::stacktrace& trace = std::stacktrace::current()
			) : Error::Boring32Error(msg, location, trace)
			{ }
	};
}
