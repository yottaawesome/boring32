module;

export module boring32.xaudio2.xaudio2error;

import boring32.error.boring32error;
import <source_location>;
import <string>;

export namespace Boring32::XAudio2
{
	class XAudio2Error : public Error::Boring32Error
	{
		public:
			virtual ~XAudio2Error();
			XAudio2Error(
				const std::source_location& location,
				const std::string& message
			);

		public:
			virtual void GenerateErrorMessage(
				const std::source_location& location,
				const std::string& message
			) override;
	};
}