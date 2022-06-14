module;

#include <source_location>
#include <string>

export module boring32.xaudio2:xaudio2error;

import boring32.error;

export namespace Boring32::XAudio2
{
	class XAudio2Error : public Error::Boring32Error
	{
		public:
			virtual ~XAudio2Error();
			XAudio2Error(
				const std::string& message,
				const std::source_location location = std::source_location::current()
			);
	};
}