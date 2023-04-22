module boring32.winhttp:winhttperror;

namespace Boring32::WinHttp
{
	WinHttpError::WinHttpError(
		const std::string& msg,
		const std::source_location location
	)
	:	Error::Boring32Error(msg, location)
	{ }
}