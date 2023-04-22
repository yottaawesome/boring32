module boring32.error:boring32error;
import :functions;

namespace Boring32::Error
{
	Boring32Error::Boring32Error()
		: m_message("Boring32 encountered an error")
	{ }

	Boring32Error::Boring32Error(
		const std::string& message,
		const std::source_location location
	)
	{
		GenerateErrorMessage(location, message);
	}

	const char* Boring32Error::what() const noexcept
	{
		return m_message.c_str();
	}

	void Boring32Error::GenerateErrorMessage(
		const std::source_location& location,
		const std::string& message
	)
	{
		m_message = Error::FormatErrorMessage("Boring32", location, message);
	}
}