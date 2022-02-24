module boring32.xaudio2.xaudio2error;
import boring32.error.functions;

namespace Boring32::XAudio2
{
	XAudio2Error::~XAudio2Error() = default;

	XAudio2Error::XAudio2Error(const XAudio2Error&) = default;
	XAudio2Error::XAudio2Error(XAudio2Error&&) noexcept = default;
	XAudio2Error& XAudio2Error::operator=(const XAudio2Error&) = default;
	XAudio2Error& XAudio2Error::operator=(XAudio2Error&&) noexcept = default;

	XAudio2Error::XAudio2Error(
		const std::source_location& location,
		const std::string& message
	) : Error::Boring32Error(location, message)
	{
		GenerateErrorMessage(location, message);
	}

	void XAudio2Error::GenerateErrorMessage(
		const std::source_location& location,
		const std::string& message
	)
	{
		m_message = Error::FormatErrorMessage("XAudio2", location, message);
	}
}