export module boring32:xaudio2.xaudio2error;
import std;
import :error;

export namespace Boring32::XAudio2
{
	class XAudio2Error final : public Error::Boring32Error
	{
	public:
		XAudio2Error(
			const std::string& message,
			const std::source_location location = std::source_location::current(),
			const std::stacktrace trace = std::stacktrace::current()
		) : Error::Boring32Error{message, location, trace}
		{ }
	};
}