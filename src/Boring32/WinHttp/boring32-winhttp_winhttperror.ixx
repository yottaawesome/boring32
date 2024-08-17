export module boring32:winhttp_winhttperror;
import boring32.shared;
import :error;

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
