module;

#include <source_location>
#include <string>

module boring32.xaudio2:xaudio2error;
import boring32.error;

namespace Boring32::XAudio2
{
	XAudio2Error::~XAudio2Error() = default;
	XAudio2Error::XAudio2Error(
		const std::source_location& location,
		const std::string& message
	) : Error::Boring32Error(location, message)
	{ }
}