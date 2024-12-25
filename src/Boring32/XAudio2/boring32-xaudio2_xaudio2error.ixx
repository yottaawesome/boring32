export module boring32:xaudio2_xaudio2error;
import std;
import :error;

export namespace Boring32::XAudio2
{
	struct XAudio2Error final : Error::Boring32Error
	{
		XAudio2Error(
			const std::string& message,
			const std::source_location location = std::source_location::current(),
			const std::stacktrace trace = std::stacktrace::current()
		) : Error::Boring32Error(message, location, trace)
		{ }
	};
}