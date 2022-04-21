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
			XAudio2Error(const XAudio2Error&);
			XAudio2Error(XAudio2Error&&) noexcept;
			XAudio2Error(
				const std::source_location& location,
				const std::string& message
			);

		public:
			virtual XAudio2Error& operator=(const XAudio2Error&);
			virtual XAudio2Error& operator=(XAudio2Error&&) noexcept;

		public:
			virtual void GenerateErrorMessage(
				const std::source_location& location,
				const std::string& message
			) override;
	};
}