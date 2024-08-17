export module boring32:xaudio2_xaudio2error;
import boring32.shared;
import :error;

export namespace Boring32::XAudio2
{
	class XAudio2Error : public Error::Boring32Error
	{
		public:
			virtual ~XAudio2Error() = default;
			XAudio2Error() = default;
			XAudio2Error(const XAudio2Error&) = default;
			XAudio2Error(XAudio2Error&&) noexcept = default;
			virtual XAudio2Error& operator=(const XAudio2Error&) = default;
			virtual XAudio2Error& operator=(XAudio2Error&&) noexcept = default;

		public:
			XAudio2Error(
				const std::string& message,
				const std::source_location location = std::source_location::current(),
				const std::stacktrace trace = std::stacktrace::current()
			) : Error::Boring32Error(message, location, trace)
			{ }
	};
}