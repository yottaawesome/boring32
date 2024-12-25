export module boring32:winhttp_winhttperror;
import std;
import :error;

export namespace Boring32::WinHttp
{
	struct WinHttpError : Error::Boring32Error
	{
		virtual ~WinHttpError() = default;
		WinHttpError(
			const std::string& msg,
			const std::source_location& location = std::source_location::current(),
			const std::stacktrace& trace = std::stacktrace::current()
		) : Error::Boring32Error(msg, location, trace)
		{ }
	};
}
