module;

#include <source_location>

module boring32.xaudio2:xaudio2error;
import boring32.error;

namespace Boring32::XAudio2
{
	XAudio2Error::~XAudio2Error() = default;
	XAudio2Error::XAudio2Error(
		const std::string& message,
		const std::source_location location
	) : Error::Boring32Error(message, location)
	{ }
}