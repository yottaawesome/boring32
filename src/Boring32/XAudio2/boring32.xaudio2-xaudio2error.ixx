export module boring32.xaudio2:xaudio2error;
import boring32.error;
import <string>;
import <source_location>;

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